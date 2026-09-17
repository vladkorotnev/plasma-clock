import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import tailwindcss from '@tailwindcss/vite';
import { musicLibrary } from './build/music-library';

export default defineConfig({
  base: './',
  plugins: [tailwindcss(), svelte(), musicLibrary()],
  worker: { format: 'es' },
  build: { target: 'es2022' },
});
