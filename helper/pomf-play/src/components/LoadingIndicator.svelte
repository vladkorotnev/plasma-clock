<script lang="ts">
  import { Spinner } from 'flowbite-svelte';
  let { label, text = '' }: { label: string; text?: string } = $props();
</script>

<span class="loading-indicator" role="status" aria-label={label}>
  <Spinner size="4" class="loading-spinner" currentColor="#dce4d6" currentFill="#628251" aria-hidden="true" />
  {#if text}<span>{text}</span>{/if}
</span>
