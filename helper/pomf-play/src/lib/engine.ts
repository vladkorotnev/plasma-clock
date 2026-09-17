import type { Mix, Mode, Snapshot, Song } from './types';
import { defaultMix } from './types';
import { CommandOpcode, LoopMarker } from './commands';
import { createVoices } from './generators';
import { GeneratorBlock } from './synthesis/block';
import { OutputMixer } from './synthesis/mixer';
import { BAUD_RATE, BLOCK_BITS, CHANNEL_COUNT } from './synthesis/constants';

/** Coordinates sequencing and streaming synthesis, retaining only one firmware block. */
export class Engine {
  voices = createVoices();
  mode: Mode = 'device'; mix = defaultMix(); loop = false; hook = false; playing = false; ended = false;
  line = 0; pointer = 0; loopPoint = 0; delay = 0; time = 0;
  private block = new GeneratorBlock(); private blockOffset = 0;
  private fraction = 0; private bits = new Float64Array(CHANNEL_COUNT + 1); private held = new Float64Array(CHANNEL_COUNT + 1);
  private mixer: OutputMixer;

  constructor(readonly song: Song, readonly sampleRate = 44100) {
    this.mixer = new OutputMixer(sampleRate);
    this.setMix(this.mix);
  }
  setMix(mix: Mix[]) { this.mix = mix; this.mixer.setMix(mix); }
  setVolume(volume: number, immediate = false) { this.mixer.setVolume(volume, immediate); }
  setHook(enabled: boolean) {
    if (enabled === this.hook) return;
    this.hook = enabled;
    if (!this.playing) { this.reset(enabled); return; }
    // Re-evaluate repeat markers without changing phase, delay, or position.
    this.loopPoint = 0;
    for (let i = 0; i < this.pointer; i++) {
      const row = this.song.commands[i];
      if (row.command === CommandOpcode.LOOP_POINT_SET
        && (row.argument === LoopMarker.LOOP || (enabled && row.argument === LoopMarker.HOOK_START))) this.loopPoint = i + 1;
    }
  }
  reset(fromHook = false) {
    this.voices = createVoices();
    this.pointer = 0; this.loopPoint = 0; this.delay = 0; this.time = 0; this.line = 0;
    this.ended = false; this.block.size = 0; this.blockOffset = 0; this.fraction = 0;
    this.mixer.reset();
    if (fromHook && this.song.hookStart >= 0) {
      // find_hook processes duty and sample assignments, skipping frequency/delay.
      for (let i = 0; i < this.song.hookStart - 1; i++) {
        const row = this.song.commands[i];
        if (row.command === CommandOpcode.DUTY_SET || row.command === CommandOpcode.SAMPLE_LOAD) this.apply(row.command, row.channel, row.argument);
      }
      this.pointer = this.song.hookStart; this.loopPoint = this.pointer;
      this.line = this.pointer; this.time = this.song.commands[this.pointer]?.time ?? 0;
    }
  }
  private apply(command: CommandOpcode, channel: number, argument: number) {
    switch (command) {
      case CommandOpcode.FREQ_SET: this.voices[channel].setFrequency(argument); break;
      case CommandOpcode.DUTY_SET: {
        const voice = this.voices[channel];
        if (voice.kind === 'square') voice.setDuty(argument);
        break;
      }
      case CommandOpcode.SAMPLE_LOAD: {
        const voice = this.voices[channel];
        if (voice.kind === 'sampler') voice.load(this.song.samples[argument], argument);
        break;
      }
    }
  }
  private finish(): boolean {
    if (this.loop) {
      this.pointer = this.loopPoint;
      this.time = this.song.commands[this.pointer]?.time ?? 0;
      return true;
    }
    this.ended = true; this.playing = false;
    for (const voice of this.voices) voice.active = false;
    return false;
  }
  private steps() {
    // Bound processing even for a malformed zero-duration looping section.
    for (let budget = this.song.commands.length + 1; budget > 0; budget--) {
      if (this.pointer >= this.song.commands.length && !this.finish()) return;
      const row = this.song.commands[this.pointer];
      if (!row) { this.ended = true; this.playing = false; return; }
      this.line = this.pointer; this.pointer++;
      this.apply(row.command, row.channel, row.argument);
      if (row.command === CommandOpcode.LOOP_POINT_SET) {
        if (row.argument === LoopMarker.HOOK_END && this.hook) { if (!this.finish()) return; }
        else if (row.argument === LoopMarker.LOOP || (row.argument === LoopMarker.HOOK_START && this.hook)) this.loopPoint = this.pointer;
      }
      if (row.command === CommandOpcode.DELAY) { this.delay = Math.trunc(row.argument * BAUD_RATE / 1000); return; }
    }
    this.ended = true; this.playing = false;
  }
  private fill() {
    if (this.delay === 0) this.steps();
    if (this.ended) { this.block.size = 0; return; }
    const wanted = Math.min(BLOCK_BITS, this.delay);
    this.block.generate(this.voices, wanted);
    this.blockOffset = 0; this.delay -= wanted;
  }
  private nextTick() {
    if (this.blockOffset >= this.block.size) this.fill();
    if (this.ended) { this.held.fill(0); return; }
    this.block.read(this.blockOffset++, this.mode, this.held);
    this.time += 1 / BAUD_RATE;
  }
  /** Integrate source bits over each host sample, then mix, DC block, and apply volume. */
  render(left: Float32Array, right: Float32Array, scopes?: Float32Array[], outputScope?: Float32Array[]) {
    const ticks = BAUD_RATE / this.sampleRate;
    let rendered = 0;
    for (let out = 0; out < left.length; out++) {
      if (!this.playing) {
        left[out] = 0; right[out] = 0;
        if (outputScope) { outputScope[0][out] = 0; outputScope[1][out] = 0; }
        continue;
      }
      this.bits.fill(0); let remaining = ticks;
      while (remaining > 1e-8) {
        if (this.fraction <= 1e-8) { this.nextTick(); this.fraction = 1; }
        const amount = Math.min(this.fraction, remaining);
        for (let channel = 0; channel <= CHANNEL_COUNT; channel++) this.bits[channel] += this.held[channel] * amount;
        this.fraction -= amount; remaining -= amount;
      }
      this.mixer.process(this.bits, ticks, this.mode);
      left[out] = this.mixer.left; right[out] = this.mixer.right;
      if (outputScope) { outputScope[0][out] = this.mixer.preLeft; outputScope[1][out] = this.mixer.preRight; }
      if (scopes) for (let channel = 0; channel < CHANNEL_COUNT; channel++) scopes[channel][out] = this.bits[channel] / ticks;
      rendered = out + 1;
    }
    return rendered;
  }
  snapshot(scopes: Float32Array[] = [], output: Float32Array[] = []): Snapshot {
    return { songId: this.song.id, playing: this.playing, ended: this.ended, line: this.line, time: this.time, scopes, output,
      voices: this.voices.map(voice => voice.status()) };
  }
}
