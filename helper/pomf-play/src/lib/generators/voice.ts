import type { VoiceStatus } from '../types';
import type { SampleMixMode } from '../commands';

/** A streaming one-bit generator, independent of packing and output mixing. */
export interface Voice {
  readonly kind: 'square' | 'noise' | 'sampler';
  active: boolean;
  frequency: number;
  readonly canGenerate: boolean;
  readonly mixMode: SampleMixMode;
  setFrequency(value: number): void;
  tick(blockOffset: number): number;
  status(): VoiceStatus;
}

export function baseStatus(voice: Voice): VoiceStatus {
  return { active: voice.active, frequency: voice.frequency, duty: 2, dutyCycle: 0.5,
    sample: -1, stretch: 1, samplePosition: 0 };
}
