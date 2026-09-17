import { SquareVoice } from './square';
import { NoiseVoice } from './noise';
import { SamplerVoice } from './sampler';
import { DEFAULT_SAMPLE } from './default-sample';

export type Voices = [SquareVoice, SquareVoice, SquareVoice, SquareVoice, NoiseVoice, SamplerVoice];
export function createVoices(): Voices {
  const sampler = new SamplerVoice();
  // NewSequencer loads the default again after Sampler's constructor.
  // Rewind deliberately preserves/toggles state, so both loads matter.
  sampler.load(DEFAULT_SAMPLE, -1);
  return [new SquareVoice(), new SquareVoice(), new SquareVoice(), new SquareVoice(), new NoiseVoice(), sampler];
}
