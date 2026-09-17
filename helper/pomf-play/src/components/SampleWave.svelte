<script lang="ts">
  import type { Sample } from '../lib/types';
  import { BAUD_RATE } from '../lib/synthesis/constants';
  let { sample, stretch, position, scaled = false, color }: { sample: Sample; stretch: number; position: number; scaled?: boolean; color: string } = $props();
  let total = $derived(sample.data.reduce((sum, run) => sum + run, 0));
  let duration = $derived(total * (scaled ? stretch / BAUD_RATE : 1 / sample.sampleRate));
  // A fixed 100 ms time axis makes integer frequency scaling visibly measurable.
  let window = $derived(scaled ? 0.1 : duration);
  let viewport = $derived(scaled ? Math.floor(position * duration / window) * 300 : 0);
  let path = $derived.by(() => {
    let time = 0; let high = true; let result = 'M0 10';
    const factor = scaled ? stretch / BAUD_RATE : 1 / sample.sampleRate;
    for (const run of sample.data) {
      const y = high ? 10 : 40; const x = time / window * 300;
      result += `L${x.toFixed(2)} ${y}`; time += run * factor;
      result += `L${(time / window * 300).toFixed(2)} ${y}`; high = !high;
    }
    return result;
  });
</script>
<div class="sample-wave">
  <svg viewBox={`${viewport} 0 300 50`} role="img" aria-label={scaled ? 'Sample waveform at current frequency with playhead' : 'Recorded sample waveform'}>
    <path d={`M${viewport} 25h300`} stroke="#dbe2d7" />
    <path d={path} fill="none" stroke={color} stroke-width="1.5" />
    {#if scaled}<path d={`M${position * duration / window * 300} 2v46`} stroke="#242d28" stroke-width="2" />{/if}
  </svg>
  <span>{scaled ? 'Scaled' : 'Recorded'} <span class="mono">{(window * 1000).toFixed(1)} ms</span></span>
</div>
