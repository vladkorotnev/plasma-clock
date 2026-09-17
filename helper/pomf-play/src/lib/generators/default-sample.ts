import type { Sample } from '../types';
import { SampleMixMode } from '../commands';

export const DEFAULT_SAMPLE: Sample = { sampleRate: 8000, rootFrequency: 524, length: 36, mode: SampleMixMode.XOR,
  data: new Uint8Array([0, 7, 4, 2, 1, 24, 23, 30, 33, 26, 38, 38, 30, 41, 41, 52, 62, 50, 58, 64, 61, 70, 99, 92, 80, 119, 102, 119, 119, 142, 146, 117, 160, 119, 154, 5]) };
