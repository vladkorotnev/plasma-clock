import { Mp3Encoder } from '@breezystack/lamejs';
import { Engine } from './engine';
import { toInt16, wavHeader } from './export';
import type { Mix, Mode, Song } from './types';

self.onmessage = ({ data }: MessageEvent<{ song: Song; mode: Mode; mix: Mix[]; volume: number; format: 'wav' | 'mp3' }>) => {
  try {
    const { song, mode, mix, volume, format } = data;
    const engine = new Engine(song, 44100); engine.mode = mode; engine.setMix(mix); engine.reset(false); engine.playing = true;
    engine.setVolume(volume, true);
    const encoder = format === 'mp3' ? new Mp3Encoder(2, 44100, 320) : undefined;
    const parts: BlobPart[] = []; let frames = 0; let lastProgress = 0;
    const left = new Float32Array(1152); const right = new Float32Array(1152);
    const l16 = new Int16Array(1152); const r16 = new Int16Array(1152);
    while (!engine.ended) {
      const count = engine.render(left, right);
      if (count === 0) break;
      frames += count;
      if (frames > 44100 * 60 * 60) throw new Error('Export exceeds the one-hour limit.');
      if (encoder) {
        for (let i = 0; i < count; i++) { l16[i] = toInt16(left[i]); r16[i] = toInt16(right[i]); }
        const encoded = encoder.encodeBuffer(l16.subarray(0, count), r16.subarray(0, count));
        if (encoded.length) parts.push(new Uint8Array(encoded).buffer);
      } else {
        const pcm = new ArrayBuffer(count * 4); const view = new DataView(pcm);
        for (let i = 0; i < count; i++) { view.setInt16(i * 4, toInt16(left[i]), true); view.setInt16(i * 4 + 2, toInt16(right[i]), true); }
        parts.push(pcm);
      }
      const progress = Math.min(0.99, engine.time / song.duration);
      if (progress - lastProgress >= 0.01) { self.postMessage({ type: 'progress', progress }); lastProgress = progress; }
    }
    if (encoder) { const tail = encoder.flush(); if (tail.length) parts.push(new Uint8Array(tail).buffer); }
    else parts.unshift(wavHeader(frames));
    const blob = new Blob(parts, { type: encoder ? 'audio/mpeg' : 'audio/wav' });
    self.postMessage({ type: 'done', blob, frames });
  } catch (error) { self.postMessage({ type: 'error', message: error instanceof Error ? error.message : String(error) }); }
};
