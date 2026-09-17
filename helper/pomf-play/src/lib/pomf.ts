import { inflateSync } from 'fflate';
import type { Command, Sample, Song } from './types';
import { CommandOpcode, LoopMarker, SampleMixMode, isCommandOpcode, commandUsesChannel } from './commands';
import { CHANNEL_COUNT } from './synthesis/constants';

const MAX_FILE = 32 * 1024 * 1024;
const MAX_EXPANDED = 64 * 1024 * 1024;
const textAt = (bytes: Uint8Array, start: number, length: number) => {
  const field = bytes.subarray(start, start + length);
  const end = field.indexOf(0);
  return new TextDecoder().decode(end < 0 ? field : field.subarray(0, end)).trim();
};

export function parsePomf(buffer: ArrayBuffer, filename = 'untitled.pomf'): Song {
  if (buffer.byteLength > MAX_FILE) throw new Error('File exceeds the 32 MiB limit.');
  const bytes = new Uint8Array(buffer);
  const view = new DataView(buffer);
  if (bytes.length < 102 || textAt(bytes, 0, 4) !== 'POmf') throw new Error('Not a PoMF binary file.');
  const version = view.getUint16(4, true);
  if (version !== 0x101 && version !== 0x100) throw new Error(`Unsupported PoMF version ${version >> 8}.${version & 255}.`);
  const samples: Sample[] = [];
  let track: Uint8Array | undefined;
  let offset = 102;
  let expanded = 0;
  let ended = false;
  while (offset < bytes.length) {
    if (offset + 12 > bytes.length) throw new Error('Truncated chunk header.');
    const magic = new TextDecoder().decode(bytes.subarray(offset, offset + 4));
    // elf2pomf.py writes ASCII sentinel words in the EOF size fields.
    if (magic === 'eof ') { ended = true; break; }
    if (!['saMP', 'saZZ', 'tuNE', 'tuZZ'].includes(magic)) throw new Error(`Unknown chunk “${magic}”.`);
    const size = view.getUint32(offset + 4, true);
    const realSize = view.getUint32(offset + 8, true);
    offset += 12;
    if (!size || size > bytes.length - offset) throw new Error(`Truncated ${magic} chunk.`);
    const compressed = magic.endsWith('ZZ');
    const expected = compressed ? realSize : size;
    expanded += expected;
    if (!expected || expanded > MAX_EXPANDED) throw new Error('Expanded file exceeds the 64 MiB limit.');
    let payload = bytes.slice(offset, offset + size);
    if (compressed) {
      try {
        // Fixed output allocation bounds malformed DEFLATE streams.
        // One extra byte detects streams larger than their declared size,
        // even though fflate truncates writes at the allocation boundary.
        payload = inflateSync(payload, { out: new Uint8Array(expected + 1) });
      } catch { throw new Error(`Invalid DEFLATE data in ${magic}.`); }
      if (payload.length !== expected) throw new Error(`Incorrect expanded size in ${magic}.`);
    }
    offset += size;
    if (magic.startsWith('sa')) {
      if (payload.length < 17) throw new Error('Truncated sample header.');
      const header = new DataView(payload.buffer, payload.byteOffset, payload.byteLength);
      const sampleRate = header.getUint16(0, true);
      const rootFrequency = header.getUint16(2, true);
      const declaredLength = header.getUint32(4, true);
      // Older files can declare more runs than their sample chunk contains.
      // The chunk boundary is authoritative; never synthesize from absent bytes.
      const length = Math.min(declaredLength, payload.length - 16);
      const mode = header.getUint32(8, true);
      if (!sampleRate || !rootFrequency || !length || (mode !== SampleMixMode.ADD && mode !== SampleMixMode.XOR))
        throw new Error('Invalid sample rate, root frequency, length, or mixing mode.');
      const data = payload.slice(16, 16 + length);
      if (!data.some(value => value > 0)) throw new Error('Sample has no nonzero runs.');
      samples.push({ sampleRate, rootFrequency, mode, data, length });
    } else {
      if (track) throw new Error('Multiple tune chunks are not supported by the firmware.');
      track = payload;
    }
  }
  if (!ended || !track || !track.length || track.length % 5) throw new Error('Missing EOF or invalid tune data (rows must be 5 bytes).');
  const data = new DataView(track.buffer, track.byteOffset, track.byteLength);
  const commands: Command[] = [];
  let duration = 0;
  let loopStart = 0;
  let hookStart = -1;
  let hookEnd = -1;
  for (let i = 0; i < track.length; i += 5) {
    const command = track[i] & 15;
    const channel = track[i] >> 4;
    const argument = data.getInt32(i + 1, true);
    const line = i / 5;
    if (!isCommandOpcode(command) || (commandUsesChannel(command) && channel >= CHANNEL_COUNT)) throw new Error(`Invalid command or channel on line ${line + 1}.`);
    if ((command === CommandOpcode.FREQ_SET || command === CommandOpcode.DELAY) && argument < 0) throw new Error(`Negative frequency or delay on line ${line + 1}.`);
    if (command === CommandOpcode.SAMPLE_LOAD && (argument < 0 || argument >= samples.length)) throw new Error(`Missing sample ${argument} on line ${line + 1}.`);
    if (command === CommandOpcode.LOOP_POINT_SET && (argument !== LoopMarker.LOOP && argument !== LoopMarker.HOOK_START && argument !== LoopMarker.HOOK_END)) throw new Error(`Unknown loop marker on line ${line + 1}.`);
    commands.push({ command, channel, argument, time: duration });
    if (command === CommandOpcode.DELAY) duration += argument / 1000;
    if (command === CommandOpcode.LOOP_POINT_SET && argument === LoopMarker.LOOP) loopStart = line + 1;
    if (command === CommandOpcode.LOOP_POINT_SET && argument === LoopMarker.HOOK_START && hookStart < 0) hookStart = line + 1;
    if (command === CommandOpcode.LOOP_POINT_SET && argument === LoopMarker.HOOK_END && hookStart >= 0 && hookEnd < 0) hookEnd = line;
  }
  if (duration === 0) throw new Error('Sequence has no positive delay.');
  return { id: crypto.randomUUID(), filename, title: textAt(bytes, 6, 32) || filename,
    longTitle: textAt(bytes, 38, 64), version, bytes: bytes.length, commands, samples, duration, loopStart, hookStart, hookEnd };
}
