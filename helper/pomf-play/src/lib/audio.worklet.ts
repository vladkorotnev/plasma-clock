import { Engine } from './engine';
import { defaultMix, defaultSnapshot, type Mix, type Mode, type Song } from './types';
import { CHANNEL_COUNT } from './synthesis/constants';

declare const sampleRate: number;
declare class AudioWorkletProcessor {
  port: MessagePort;
  constructor();
}
declare function registerProcessor(name: string, processor: typeof AudioWorkletProcessor): void;

class PomfProcessor extends AudioWorkletProcessor {
  private ids: string[] = []; private next?: Song; private engine?: Engine;
  private mode: Mode = 'device'; private mix = defaultMix(); private loop = false; private hook = false;
  private volume = 1;
  private frames = 0; private scopes = Array.from({ length: CHANNEL_COUNT }, () => new Float32Array(512));
  private outputScopes = Array.from({ length: 2 }, () => new Float32Array(512));
  private scopeWrite = 0; private outputWrite = 0;
  constructor() {
    super();
    this.port.onmessage = ({ data }) => {
      try {
        if (data.type === 'clear') {
          this.engine = undefined; this.next = undefined;
          this.port.postMessage({ type: 'snapshot', snapshot: defaultSnapshot() });
          return;
        }
        if (data.type === 'playlist') this.ids = data.ids;
        if (data.type === 'next') this.next = data.song;
        if (data.type === 'load') {
          this.start(data.song, data.playing, data.fromHook);
        } else if (data.type === 'play' && this.engine) {
          if (this.engine.ended) this.engine.reset(this.hook);
          this.engine.playing = true;
        } else if (data.type === 'pause' && this.engine) this.engine.playing = false;
        else if (data.type === 'rewind' && this.engine) this.engine.reset(false);
        else if (data.type === 'settings') {
          this.mode = data.mode; this.mix = data.mix as Mix[]; this.loop = data.loop;
          const changedVolume = this.volume !== data.volume; this.volume = data.volume;
          const changedHook = this.hook !== data.hook; this.hook = data.hook;
          if (this.engine) {
            this.engine.mode = this.mode; this.engine.setMix(this.mix); this.engine.loop = this.loop;
            if (changedVolume) this.engine.setVolume(this.volume);
            if (changedHook) this.engine.setHook(this.hook);
          }
        }
        this.report();
      } catch (error) { this.port.postMessage({ type: 'error', message: String(error) }); }
    };
  }
  private start(song: Song, playing: boolean, fromHook = this.hook) {
    this.engine = new Engine(song, sampleRate);
    this.engine.mode = this.mode; this.engine.setMix(this.mix); this.engine.loop = this.loop; this.engine.hook = this.hook;
    this.engine.setVolume(this.volume, true);
    this.engine.reset(fromHook); this.engine.playing = playing;
    this.scopes.forEach(scope => scope.fill(0)); this.scopeWrite = 0;
    this.outputScopes.forEach(scope => scope.fill(0)); this.outputWrite = 0;
  }
  private report() {
    if (!this.engine) return;
    const ordered = [...this.scopes, ...this.outputScopes].map((scope, channel) => {
      const write = channel < CHANNEL_COUNT ? this.scopeWrite : this.outputWrite;
      const result = new Float32Array(scope.length);
      result.set(scope.subarray(write)); result.set(scope.subarray(0, write), scope.length - write);
      return result;
    });
    this.port.postMessage({ type: 'snapshot', snapshot: this.engine.snapshot(ordered.slice(0, CHANNEL_COUNT), ordered.slice(CHANNEL_COUNT)) }, ordered.map(scope => scope.buffer));
  }
  process(_inputs: Float32Array[][], outputs: Float32Array[][]) {
    const output = outputs[0]; if (!output?.[0] || !this.engine) return true;
    const left = output[0]; const right = output[1];
    const mixedView = this.outputScopes.map(scope => scope.subarray(this.outputWrite, this.outputWrite + left.length));
    mixedView.forEach(scope => scope.fill(0));
    let ended = false;
    if (this.engine.playing) {
      const view = this.scopes.map(scope => scope.subarray(this.scopeWrite, this.scopeWrite + left.length));
      view.forEach(scope => scope.fill(0));
      const rendered = this.engine.render(left, right, view, mixedView);
      this.scopeWrite = (this.scopeWrite + left.length) % 512;
      if (this.engine.ended) {
        ended = true;
        const index = this.ids.indexOf(this.engine.song.id);
        const nextId = this.ids[index + 1];
        const next = this.next?.id === nextId ? this.next : undefined;
        if (next) {
          // Preserve this quantum's pre-fader samples across the new song's scope reset.
          const previousMix = mixedView.map(scope => scope.slice(0, rendered));
          this.start(next, true); this.next = undefined;
          const nextMix = this.outputScopes.map(scope => scope.subarray(0, left.length));
          nextMix.forEach((scope, channel) => scope.set(previousMix[channel]));
          if (rendered < left.length) this.engine.render(left.subarray(rendered), right.subarray(rendered), undefined, nextMix.map(scope => scope.subarray(rendered)));
        } else if (nextId) {
          this.port.postMessage({ type: 'advance', from: this.engine.song.id, id: nextId });
        }
      }
    }
    this.outputWrite = (this.outputWrite + left.length) % 512;
    this.frames += left.length;
    if (ended || this.frames >= sampleRate / 60) { this.frames = 0; this.report(); }
    return true;
  }
}
registerProcessor('pomf-player', PomfProcessor);
