<script lang="ts">
  import type { Mix, Mode, Sample, VoiceStatus } from '../lib/types';
  import { DEFAULT_SAMPLE } from '../lib/generators/default-sample';
  import Oscilloscope from './Oscilloscope.svelte';
  import Piano from './Piano.svelte';
  import SampleWave from './SampleWave.svelte';
  let { channel, voice, scope, mix, mode, samples, onchange }: {
    channel: number; voice: VoiceStatus; scope?: Float32Array; mix: Mix; mode: Mode; samples: Sample[];
    onchange: (channel: number, field: 'volume' | 'pan', value: number) => void;
  } = $props();
  const colors = ['#52735b', '#648294', '#a47d4a', '#877598', '#798264', '#a17060'];
  let color = $derived(colors[channel]);
  let sample = $derived(samples[voice.sample] ?? DEFAULT_SAMPLE);
  let duty = $derived(voice.dutyCycle);
</script>
<article class="track" class:noise={channel === 4} style={`--track-color:${color}`}>
  <div class="track-controls">
    <div class="track-heading"><h3><span class="channel-number" class:active={voice.active} aria-label={`Channel ${channel} ${voice.active ? 'active' : 'inactive'}`}>{channel.toString().padStart(2, '0')}</span>{channel < 4 ? 'Square' : channel === 4 ? 'Noise' : 'Sampler'}</h3></div>
    <div class="mix-controls" class:locked={mode === 'device'}>
      <label>Volume <input type="range" min="0" max="1" step="0.01" value={mix.volume} disabled={mode === 'device'} oninput={e => onchange(channel, 'volume', +e.currentTarget.value)} aria-label={`Channel ${channel} volume`} /><output class="mono">{Math.round(mix.volume * 100)}%</output></label>
      <label>Pan <input type="range" min="-1" max="1" step="0.01" value={mix.pan} disabled={mode === 'device'} oninput={e => onchange(channel, 'pan', +e.currentTarget.value)} aria-label={`Channel ${channel} pan`} /><output class="mono">{Math.abs(mix.pan) < 0.01 ? 'C' : `${mix.pan < 0 ? 'L' : 'R'}${Math.round(Math.abs(mix.pan) * 100)}`}</output></label>
    </div>
  </div>
  <div class="track-readout"><span class="frequency mono">{voice.active ? voice.frequency.toLocaleString() : '—'} <small>Hz</small></span>
    {#if channel < 4}<span>Duty <strong class="mono">{(duty * 100).toFixed(1)}%</strong></span>{/if}
    {#if channel === 5}<span>Sample <strong class="mono">{voice.sample < 0 ? 'DEFAULT' : voice.sample}</strong></span>{/if}
  </div>
  {#if channel !== 4}<div class="track-piano"><Piano frequency={voice.frequency} active={voice.active} {color} /></div>{/if}
  <Oscilloscope data={scope} {color} active={voice.active} />
  {#if channel === 5}<div class="sample-pair"><SampleWave {sample} stretch={voice.stretch} position={voice.samplePosition} {color} /><SampleWave {sample} stretch={voice.stretch} position={voice.samplePosition} {color} scaled /></div>{/if}
</article>
