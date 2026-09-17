import type { Mix, Mode } from '../types';
import { CHANNEL_COUNT, DEVICE_GAIN } from './constants';

/** Realtime stereo mix, AC coupling, and master-fader ramp; no per-sample allocations. */
export class OutputMixer {
  left = 0; right = 0; preLeft = 0; preRight = 0;
  private previous = [0, 0]; private filtered = [0, 0];
  private panLeft = new Float64Array(CHANNEL_COUNT); private panRight = new Float64Array(CHANNEL_COUNT);
  private masterVolume = 1; private outputGain = 1; private gainStep = 0; private gainSamples = 0;
  private readonly dc: number;
  constructor(readonly sampleRate: number) { this.dc = Math.exp(-2 * Math.PI * 20 / sampleRate); }
  reset() { this.previous = [0, 0]; this.filtered = [0, 0]; }
  setMix(mix: Mix[]) {
    for (let channel = 0; channel < CHANNEL_COUNT; channel++) {
      const angle = (mix[channel].pan + 1) * Math.PI / 4;
      this.panLeft[channel] = Math.cos(angle) * mix[channel].volume / CHANNEL_COUNT;
      this.panRight[channel] = Math.sin(angle) * mix[channel].volume / CHANNEL_COUNT;
    }
  }
  setVolume(volume: number, immediate = false) {
    this.masterVolume = Math.max(0, Math.min(1, volume));
    this.gainSamples = immediate ? 0 : Math.ceil(this.sampleRate * 0.005);
    this.gainStep = this.gainSamples ? (this.masterVolume - this.outputGain) / this.gainSamples : 0;
    if (immediate) this.outputGain = this.masterVolume;
  }
  process(bits: Float64Array, ticks: number, mode: Mode) {
    let left = 0; let right = 0;
    if (mode === 'device') left = right = bits[CHANNEL_COUNT] / ticks * DEVICE_GAIN;
    else for (let channel = 0; channel < CHANNEL_COUNT; channel++) {
      left += bits[channel] / ticks * this.panLeft[channel]; right += bits[channel] / ticks * this.panRight[channel];
    }
    for (let channel = 0; channel < 2; channel++) {
      const value = channel ? right : left;
      this.filtered[channel] = value - this.previous[channel] + this.dc * this.filtered[channel];
      this.previous[channel] = value;
    }
    if (this.gainSamples > 0) { this.outputGain += this.gainStep; this.gainSamples--; }
    else this.outputGain = this.masterVolume;
    this.preLeft = this.filtered[0]; this.preRight = this.filtered[1];
    this.left = this.preLeft * this.outputGain; this.right = this.preRight * this.outputGain;
  }
}
