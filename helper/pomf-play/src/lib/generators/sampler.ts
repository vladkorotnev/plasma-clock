import { BAUD_RATE } from '../synthesis/constants';
import type { Sample, VoiceStatus } from '../types';
import { baseStatus, type Voice } from './voice';
import { DEFAULT_SAMPLE } from './default-sample';

export class SamplerVoice implements Voice {
  readonly kind = 'sampler';
  active = false; state = false; frequency = 0;
  sample = DEFAULT_SAMPLE; sampleIndex = -1; playhead = 0; remaining = 0; stretch = 1;
  constructor() { this.load(DEFAULT_SAMPLE, -1); }
  get canGenerate() { return this.active; }
  get mixMode() { return this.sample.mode; }

  rewind() {
    this.playhead = 0; this.remaining = this.sample.data[0];
    while (!this.remaining && this.playhead + 1 < this.sample.length) {
      this.playhead++; this.remaining = this.sample.data[this.playhead]; this.state = !this.state;
    }
  }
  load(sample: Sample, index: number) { this.sample = sample; this.sampleIndex = index; this.rewind(); this.stretch = 1; }
  setFrequency(value: number) {
    this.active = value !== 0;
    if (value) {
      this.frequency = value;
      this.stretch = Math.max(1, Math.trunc(Math.trunc(BAUD_RATE / this.sample.sampleRate) * this.sample.rootFrequency / value));
      this.rewind();
    }
  }
  tick(blockOffset: number): number {
    if (!this.canGenerate) return 0;
    const output = +this.state;
    // The firmware restarts the stretch counter at every fill_buffer call.
    if (this.stretch === 1 || (blockOffset > 0 && blockOffset % this.stretch === 0)) {
      this.remaining--;
      if (this.remaining === 0) {
        this.playhead = (this.playhead + 1) % this.sample.length;
        this.remaining = this.sample.data[this.playhead]; this.state = !this.state;
      }
    }
    return output;
  }
  position() {
    let before = 0; let total = 0;
    for (let i = 0; i < this.sample.length; i++) { total += this.sample.data[i]; if (i < this.playhead) before += this.sample.data[i]; }
    return total ? Math.max(0, Math.min(1, (before + this.sample.data[this.playhead] - this.remaining) / total)) : 0;
  }
  status(): VoiceStatus {
    return { ...baseStatus(this), sample: this.sampleIndex, stretch: this.stretch, samplePosition: this.position() };
  }
}
