export function wavHeader(frames: number, sampleRate = 44100) {
  const buffer = new ArrayBuffer(44); const view = new DataView(buffer);
  const text = (offset: number, value: string) => { [...value].forEach((char, i) => view.setUint8(offset + i, char.charCodeAt(0))); };
  text(0, 'RIFF'); view.setUint32(4, 36 + frames * 4, true); text(8, 'WAVE'); text(12, 'fmt ');
  view.setUint32(16, 16, true); view.setUint16(20, 1, true); view.setUint16(22, 2, true);
  view.setUint32(24, sampleRate, true); view.setUint32(28, sampleRate * 4, true); view.setUint16(32, 4, true);
  view.setUint16(34, 16, true); text(36, 'data'); view.setUint32(40, frames * 4, true);
  return buffer;
}
export const toInt16 = (value: number) => Math.round(Math.max(-1, Math.min(1, value)) * (value < 0 ? 32768 : 32767));
