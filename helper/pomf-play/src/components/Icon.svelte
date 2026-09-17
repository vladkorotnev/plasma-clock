<script lang="ts">
  let { name, size = 18 }: { name: string; size?: number } = $props();
  const paths: Record<string, string> = {
    play: 'M8 5v14l11-7z', pause: 'M8 5v14M16 5v14', rewind: 'M5 5v14M19 5l-10 7 10 7z',
    previous: 'M5 5v14M18 5l-9 7 9 7z', next: 'M19 5v14M6 5l9 7-9 7z',
    plus: 'M12 5v14M5 12h14', close: 'M6 6l12 12M6 18L18 6',
    up: 'M6 14l6-6 6 6', down: 'M6 10l6 6 6-6',
    download: 'M12 3v12M7 10l5 5 5-5M5 16v5h14v-5',
    hook: 'M8 4v10a5 5 0 0 0 10 0v-3M4 8l4-4 4 4',
    wave: 'M2 16V8h6v8h8V8h6', folder: 'M3 6h7l2 3h9v11H3z',
  };
</script>
<svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d={paths[name] ?? paths.wave} /></svg>
