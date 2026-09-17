export interface Note { midi: number; pitchClass: number; octave: number; name: string }
const names = ['C', 'C♯', 'D', 'D♯', 'E', 'F', 'F♯', 'G', 'G♯', 'A', 'A♯', 'B'];
export const pitchClass = (midi: number) => ((midi % 12) + 12) % 12;
export function frequencyToNote(frequency: number): Note | null {
  if (!Number.isFinite(frequency) || frequency <= 0) return null;
  const midi = Math.round(69 + 12 * Math.log2(frequency / 440));
  const pitch = pitchClass(midi); const octave = Math.floor(midi / 12) - 1;
  return { midi, pitchClass: pitch, octave, name: `${names[pitch]}${octave}` };
}
