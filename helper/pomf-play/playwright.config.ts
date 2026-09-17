import { defineConfig } from '@playwright/test';
export default defineConfig({
  testDir: './test/browser',
  use: { baseURL: 'http://127.0.0.1:4173/pomfplay/', headless: true, launchOptions: { args: ['--autoplay-policy=no-user-gesture-required'] } },
  webServer: { command: 'node test/static-server.mjs', url: 'http://127.0.0.1:4173/pomfplay/', reuseExistingServer: true },
});
