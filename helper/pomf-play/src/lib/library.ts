import { parsePomf } from './pomf';
import type { PlaylistEntry, Song, SongInfo } from './types';

export function songInfo(song: Song): SongInfo {
  const { commands, samples, ...info } = song;
  return { ...info, commandCount: commands.length, sampleCount: samples.length,
    loopTime: commands[song.loopStart]?.time ?? song.duration,
    hookStartTime: commands[song.hookStart]?.time ?? song.duration,
    hookEndTime: commands[song.hookEnd]?.time ?? song.duration };
}

export async function loadMusicLibrary(signal: AbortSignal): Promise<PlaylistEntry[]> {
  const catalogUrl = new URL('music/index.json', document.baseURI);
  const response = await fetch(catalogUrl, { signal });
  if (!response.ok) throw new Error(`Could not load the music catalog (HTTP ${response.status}).`);
  const catalog: unknown = await response.json();
  if (!Array.isArray(catalog) || !catalog.every(info => info && typeof info.filename === 'string'
    && /\.pomf$/i.test(info.filename) && !info.filename.includes('/') && !info.filename.includes('\\')
    && typeof info.title === 'string' && typeof info.longTitle === 'string'
    && ['version', 'bytes', 'duration', 'loopStart', 'hookStart', 'hookEnd', 'commandCount', 'sampleCount',
      'loopTime', 'hookStartTime', 'hookEndTime'].every(key => Number.isFinite(info[key])))) {
    throw new Error('Invalid music catalog.');
  }
  return catalog.map(info => ({ ...info, id: crypto.randomUUID(),
    url: new URL(encodeURIComponent(info.filename), catalogUrl).href }));
}

/** Local files stay resident; only the current and next remote songs are retained. */
export class SongLoader {
  private retained = new Set<string>();
  private cache = new Map<string, Song>();
  private pending = new Map<string, Promise<Song>>();
  constructor(private signal: AbortSignal, private onLoading: (ids: string[]) => void = () => {}) {}
  retain(ids: string[]) {
    this.retained = new Set(ids);
    for (const id of this.cache.keys()) if (!this.retained.has(id)) this.cache.delete(id);
  }
  load(entry: PlaylistEntry): Promise<Song> {
    if (entry.local) return Promise.resolve(entry.local);
    const cached = this.cache.get(entry.id);
    if (cached) return Promise.resolve(cached);
    const pending = this.pending.get(entry.id);
    if (pending) return pending;
    const request = (async () => {
      const response = await fetch(entry.url!, { signal: this.signal });
      if (!response.ok) throw new Error(`${entry.filename}: HTTP ${response.status}`);
      let song: Song;
      try { song = { ...parsePomf(await response.arrayBuffer(), entry.filename), id: entry.id }; }
      catch (cause) { throw new Error(`${entry.filename}: ${cause instanceof Error ? cause.message : String(cause)}`); }
      if (this.retained.has(entry.id)) this.cache.set(entry.id, song);
      return song;
    })();
    this.pending.set(entry.id, request);
    this.onLoading([...this.pending.keys()]);
    void request.finally(() => {
      this.pending.delete(entry.id); this.onLoading([...this.pending.keys()]);
    }).catch(() => {});
    return request;
  }
}
