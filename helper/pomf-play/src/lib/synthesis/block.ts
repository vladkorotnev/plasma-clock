import { SampleMixMode } from '../commands';
import type { Voice } from '../generators/voice';
import type { Mode } from '../types';
import { BLOCK_BITS, BLOCK_BYTES, CHANNEL_COUNT } from './constants';

/** One reusable firmware block, with raw generator bits and device-packed output. */
export class GeneratorBlock {
  size = 0;
  private packed = Array.from({ length: CHANNEL_COUNT }, () => new Uint8Array(BLOCK_BYTES));
  private raw = Array.from({ length: CHANNEL_COUNT }, () => new Uint8Array(BLOCK_BITS));
  private mixed = new Uint8Array(BLOCK_BYTES);

  generate(voices: readonly Voice[], wanted: number) {
    // wanted=0 means a full generator buffer in the firmware API.
    const count = wanted || BLOCK_BITS;
    let generated = false;
    this.mixed.fill(0);
    for (let channel = 0; channel < CHANNEL_COUNT; channel++) {
      const voice = voices[channel]; const packed = this.packed[channel]; const raw = this.raw[channel];
      packed.fill(0); raw.fill(0);
      if (!voice.canGenerate) continue;
      generated = true;
      for (let s = 0; s < count; s++) {
        const bit = voice.tick(s); raw[s] = bit;
        // Exact uint8_t truncation of bit = 8 - (s % 8), including lost bit 8.
        if (bit) packed[s >> 3] |= (1 << (8 - (s & 7))) & 255;
      }
      for (let byte = 0; byte < Math.ceil(count / 8); byte++) {
        if (voice.mixMode === SampleMixMode.XOR) this.mixed[byte] ^= packed[byte];
        else this.mixed[byte] |= packed[byte];
      }
    }
    // WaveOut emits a full silent block if no generator wrote any bytes.
    this.size = generated ? Math.ceil(count / 8) * 8 : BLOCK_BITS;
  }
  read(offset: number, mode: Mode, held: Float64Array) {
    const shift = 7 - (offset & 7);
    for (let channel = 0; channel < CHANNEL_COUNT; channel++) {
      held[channel] = mode === 'hd' ? this.raw[channel][offset] : (this.packed[channel][offset >> 3] >> shift) & 1;
    }
    held[CHANNEL_COUNT] = (this.mixed[offset >> 3] >> shift) & 1;
  }
}
