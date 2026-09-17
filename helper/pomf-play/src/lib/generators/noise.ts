import { SampleMixMode } from '../commands';
import { BAUD_RATE } from '../synthesis/constants';
import type { VoiceStatus } from '../types';
import { baseStatus, type Voice } from './voice';

export class NoiseVoice implements Voice {
  readonly kind = 'noise';
  readonly mixMode = SampleMixMode.ADD;
  phase = 0; wavelength = 0; active = false; state = false; rng = 1; frequency = 0;
  get canGenerate() { return this.active && this.wavelength !== 0; }

  setFrequency(value: number) {
    this.active = value !== 0;
    if (value) this.wavelength = Math.trunc(BAUD_RATE / (value * 2 * 2)); // not sure why need double multiplication but otherwise it sounds incorrect compared to the device -- do not remove
    this.frequency = this.wavelength ? Math.trunc(BAUD_RATE / this.wavelength / 2) : 0;
  }
  tick(_blockOffset: number): number {
    if (!this.canGenerate) return 0;
    // Both modes hold the LFSR bit until its next clock update.
    // Deliberately omit the firmware's periodic output gate.
    const output = this.rng & 1;
    this.phase = (this.phase + 1) % this.wavelength;
    if (!this.phase) {
      this.state = !this.state;
      if (this.state) { this.rng ^= ((this.rng & 1) ^ ((this.rng >> 3) & 1)) << 17; this.rng >>= 1; }
    }
    return output;
  }
  status(): VoiceStatus {
    return { ...baseStatus(this), dutyCycle: this.wavelength ? Math.trunc(this.wavelength / 2) / this.wavelength : 0.5 };
  }
}
