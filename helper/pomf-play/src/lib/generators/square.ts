import { SampleMixMode } from '../commands';
import { BAUD_RATE } from '../synthesis/constants';
import type { VoiceStatus } from '../types';
import { baseStatus, type Voice } from './voice';

export class SquareVoice implements Voice {
  readonly kind = 'square';
  readonly mixMode = SampleMixMode.ADD;
  phase = 0; wavelength = 0; duty = 2; active = false; frequency = 0;
  get canGenerate() { return this.active && this.wavelength !== 0; }

  setFrequency(value: number) {
    if (value > 0 && value <= BAUD_RATE / 2) this.wavelength = Math.trunc(BAUD_RATE / value);
    this.phase = 0; this.active = value !== 0;
    this.frequency = this.wavelength ? Math.trunc(BAUD_RATE / this.wavelength) : 0;
  }
  setDuty(value: number) { this.duty = Math.abs(value) < 2 ? 2 : value; }
  tick(_blockOffset: number): number {
    if (!this.canGenerate) return 0;
    let high = Math.trunc(this.wavelength / Math.abs(this.duty));
    if (this.duty < 0) high = this.wavelength - high;
    const output = +(this.phase < high);
    this.phase = (this.phase + 1) % this.wavelength;
    return output;
  }
  status(): VoiceStatus {
    return { ...baseStatus(this), duty: this.duty,
      dutyCycle: this.wavelength ? (this.duty < 0 ? 1 - Math.trunc(this.wavelength / Math.abs(this.duty)) / this.wavelength
        : Math.trunc(this.wavelength / Math.abs(this.duty)) / this.wavelength) : 0.5 };
  }
}
