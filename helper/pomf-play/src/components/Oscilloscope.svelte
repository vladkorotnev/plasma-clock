<script lang="ts">
  let { data, rightData, color, active, bipolar = false, gain = 1, label = 'Generator output before volume and pan' }: {
    data?: Float32Array; rightData?: Float32Array; color: string; active: boolean; bipolar?: boolean; gain?: number; label?: string;
  } = $props();
  let canvas: HTMLCanvasElement;
  let traceColor = $derived.by(() => {
    const rgb = Number.parseInt(color.slice(1), 16);
    const channels = [16, 8, 0].map(shift => {
      const value = (rgb >> shift) & 255;
      return Math.round(value + (255 - value) * 0.45);
    });
    return `rgb(${channels.join(',')})`;
  });
  $effect(() => {
    const signals = rightData ? [data, rightData] : [data]; const enabled = active; const stroke = traceColor; const centered = bipolar; const scale = gain;
    if (!canvas) return;
    const ctx = canvas.getContext('2d'); if (!ctx) return;
    const w = 600; const h = 92;
    ctx.shadowBlur = 0; ctx.globalAlpha = 1;
    ctx.fillStyle = '#000'; ctx.fillRect(0, 0, w, h);
    ctx.strokeStyle = '#17201b'; ctx.lineWidth = 1;
    ctx.beginPath();
    for (let x = 0; x <= w; x += 50) { ctx.moveTo(x, 0); ctx.lineTo(x, h); }
    for (let y = 16; y < h; y += 30) { ctx.moveTo(0, y); ctx.lineTo(w, y); }
    ctx.stroke(); ctx.strokeStyle = enabled ? stroke : '#4f5d54'; ctx.lineWidth = 2;
    ctx.shadowColor = stroke; ctx.shadowBlur = enabled ? 8 : 0;
    for (const [channel, signal] of signals.entries()) {
      ctx.globalAlpha = channel ? 0.5 : 1;
      ctx.beginPath();
      const length = signal?.length ?? 2;
      for (let i = 0; i < length; i++) {
        const x = i / (length - 1) * w;
        const value = enabled && signal ? Math.max(-1, Math.min(1, signal[i] * scale)) : 0;
        const y = centered ? h / 2 - value * (h / 2 - 8) : enabled && signal ? h - 18 - value * (h - 36) : h / 2;
        if (!i) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
    ctx.globalAlpha = 1; ctx.shadowBlur = 0;
  });
</script>
<canvas bind:this={canvas} width="600" height="92" class="scope" aria-label={label}></canvas>
