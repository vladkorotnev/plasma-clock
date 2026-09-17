import { readFile, readdir } from 'node:fs/promises';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';
import type { Plugin } from 'vite';
import { parsePomf } from '../src/lib/pomf';
import { songInfo } from '../src/lib/library';

const directory = fileURLToPath(new URL('../../../data/music/', import.meta.url));
async function filenames(): Promise<string[]> {
  const entries = await readdir(directory, { withFileTypes: true });
  return entries.filter(entry => entry.isFile() && /\.pomf$/i.test(entry.name)).map(entry => entry.name).sort();
}

/** Publish the repository's binary files and a catalog alongside the static player. */
export function musicLibrary(): Plugin {
  let building = false;
  return {
    name: 'pomf-music-library',
    configResolved(config) { building = config.command === 'build'; },
    async buildStart() {
      if (!building) return;
      this.addWatchFile(directory);
      const names = await filenames();
      const contents = await Promise.all(names.map(name => readFile(join(directory, name))));
      for (const [index, name] of names.entries()) {
        this.addWatchFile(join(directory, name));
        this.emitFile({ type: 'asset', fileName: `music/${name}`, source: contents[index] });
      }
      this.emitFile({ type: 'asset', fileName: 'music/index.json', source: JSON.stringify(contents.map((bytes, index) => {
        const { id, ...info } = songInfo(parsePomf(Uint8Array.from(bytes).buffer, names[index]));
        return info;
      })) });
    },
    configureServer(server) {
      server.middlewares.use('/music', async (request, response, next) => {
        if (request.method !== 'GET' && request.method !== 'HEAD') { next(); return; }
        try {
          const name = decodeURIComponent(new URL(request.url ?? '/', 'http://localhost').pathname.slice(1));
          const names = await filenames();
          if (name === 'index.json') {
            const catalog = await Promise.all(names.map(async filename => {
              const bytes = await readFile(join(directory, filename));
              const { id, ...info } = songInfo(parsePomf(Uint8Array.from(bytes).buffer, filename));
              return info;
            }));
            response.writeHead(200, { 'Content-Type': 'application/json', 'Cache-Control': 'no-cache' });
            response.end(request.method === 'HEAD' ? undefined : JSON.stringify(catalog));
          } else if (names.includes(name)) {
            const contents = await readFile(join(directory, name));
            response.writeHead(200, { 'Content-Type': 'application/octet-stream', 'Cache-Control': 'no-cache' });
            response.end(request.method === 'HEAD' ? undefined : contents);
          } else response.writeHead(404).end();
        } catch { response.writeHead(500).end('Could not load bundled music.'); }
      });
    },
  };
}
