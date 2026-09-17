<script lang="ts">
  import { Select } from 'flowbite-svelte';
  import type { Mode } from '../lib/types';
  import Oscilloscope from './Oscilloscope.svelte';
  let { mode = $bindable(), output, playing }: { mode: Mode; output: Float32Array[]; playing: boolean } = $props();
  const modes = [{ value: 'device', name: 'Device accurate' }, { value: 'hd', name: 'High definition' }];
</script>
<article class="track mixed-track" style="--track-color:#345c4a">
  <div class="track-controls">
    <div class="track-heading"><h3><span class="channel-number" class:active={playing}>Σ</span>Mixed audio</h3></div>
    <Select items={modes} bind:value={mode} aria-label="Output" class="output-select" />
  </div>
  <Oscilloscope data={output[0]} rightData={output[1]} color="#345c4a" active={playing} bipolar gain={5} label="Mixed stereo output before master volume" />
</article>
