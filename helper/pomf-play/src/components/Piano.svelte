<script lang="ts">
  import { frequencyToNote, pitchClass } from '../lib/notes';
  let { frequency, active, color }: { frequency: number; active: boolean; color: string } = $props();
  let resolved = $derived(frequencyToNote(frequency));
  let midi = $derived(resolved?.midi ?? -1);
  let start = $derived(Math.floor((resolved?.midi ?? 60) / 12) * 12 - 12);
  const offsets = [0, 0.7, 1, 1.7, 2, 3, 3.7, 4, 4.7, 5, 5.7, 6];
  const black = (note: number) => [1, 3, 6, 8, 10].includes(pitchClass(note));
  let keys = $derived(Array.from({ length: 24 }, (_, i) => start + i));
  let note = $derived(active ? resolved?.name ?? '—' : '—');
</script>
<div class="piano-wrap">
  <svg class="piano" viewBox="0 0 280 34" role="img" aria-label={`Current note ${note}`}>
    {#each keys.filter(key => !black(key)) as key}
      <rect x={((Math.floor((key - start) / 12) * 7) + offsets[pitchClass(key)]) * 20 + 0.5} y="0.5" width="19" height="32" rx="2" fill={active && key === midi ? color : '#f5f6f2'} stroke="#cfd3cc" />
      {#if key % 12 === 0}<text x={((key - start) / 12 * 7) * 20 + 10} y="28" text-anchor="middle" font-size="7" fill="#626a61">C{Math.floor(key / 12) - 1}</text>{/if}
    {/each}
    {#each keys.filter(black) as key}
      <rect x={((Math.floor((key - start) / 12) * 7) + offsets[pitchClass(key)]) * 20 - 3} y="0" width="12" height="20" rx="1.5" fill={active && key === midi ? color : '#3e4843'} />
    {/each}
  </svg>
  <span class="note-value">{note}</span>
</div>
