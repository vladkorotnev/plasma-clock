import workletUrl from './audio.worklet.ts?worker&url';
import type { Mix, Mode, Snapshot, Song } from './types';
import { defaultMix, defaultSnapshot } from './types';

export class AudioPlayer {
  private context?: AudioContext; private node?: AudioWorkletNode; private initializing?: Promise<void>;
  private ids: string[] = []; private selected = ''; private song?: Song; private next?: Song;
  private revision = 0; private wantsPlayback = false; private closed = false;
  private settings = { mode: 'device' as Mode, mix: defaultMix(), loop: false, hook: false, volume: 1 };
  constructor(private resolve: (id: string) => Promise<Song>, private onSong: (song: Song) => void,
    private onSnapshot: (snapshot: Snapshot) => void, private onError: (message: string) => void) {}
  private async ensure() {
    if (!this.initializing) this.initializing = (async () => {
      if (!window.AudioContext) throw new Error('This browser does not support Web Audio.');
      this.context = new AudioContext({ latencyHint: 'interactive', sampleRate: 44100 });
      if (!this.context.audioWorklet) throw new Error('AudioWorklet requires HTTPS or localhost.');
      await this.context.audioWorklet.addModule(workletUrl);
      if (this.closed) return;
      this.node = new AudioWorkletNode(this.context, 'pomf-player', { numberOfInputs: 0, numberOfOutputs: 1, outputChannelCount: [2] });
      this.node.port.onmessage = ({ data }) => {
        if (data.type === 'snapshot') {
          const status = data.snapshot as Snapshot;
          if (status.songId && status.songId !== this.selected) {
            if (status.songId !== this.next?.id) return;
            this.selected = status.songId; this.song = this.next; this.next = undefined; this.revision++;
            this.onSong(this.song);
            this.onSnapshot(status); if (this.wantsPlayback) void this.prepareNext(); return;
          }
          if (!status.songId && this.selected) return;
          const waiting = status.ended && Boolean(this.ids[this.ids.indexOf(this.selected) + 1]);
          if (status.ended && !waiting) this.wantsPlayback = false;
          this.onSnapshot(waiting ? { ...status, playing: this.wantsPlayback } : status);
        }
        if (data.type === 'advance' && data.from === this.selected) {
          // If a slow request missed the boundary, continue as soon as it finishes.
          void this.advance(data.id);
        }
        if (data.type === 'error') this.onError(data.message);
      };
      this.node.onprocessorerror = () => this.onError('The audio processor stopped. Reload the page to restart it.');
      this.node.connect(this.context.destination);
      this.send({ type: 'playlist', ids: this.ids });
      this.send({ type: 'settings', ...this.settings });
      if (this.song) this.send({ type: 'load', song: this.song, playing: false, fromHook: this.settings.hook });
      if (this.next) this.send({ type: 'next', song: this.next });
    })().catch(error => { this.initializing = undefined; void this.context?.close(); throw error; });
    await this.initializing;
    await this.context!.resume();
  }
  private send(message: object) { this.node?.port.postMessage(message); }
  playlist(ids: string[]) {
    this.ids = ids; this.next = undefined;
    this.send({ type: 'playlist', ids }); this.send({ type: 'next', song: undefined });
    if (this.wantsPlayback) void this.prepareNext();
  }
  async select(id: string, playing = false, load = true) {
    const revision = ++this.revision;
    this.selected = id; this.song = undefined; this.next = undefined; this.wantsPlayback = playing;
    this.send({ type: 'clear' });
    if (!id || !load) return;
    try {
      // Start Web Audio from the user gesture, before awaiting a network fetch.
      const ready = playing ? this.ensure() : Promise.resolve();
      const [song] = await Promise.all([this.resolve(id), ready]);
      if (revision !== this.revision || this.closed) return;
      this.song = song; this.onSong(song);
      this.send({ type: 'load', song, playing: this.wantsPlayback, fromHook: this.settings.hook });
      if (this.wantsPlayback) void this.prepareNext();
    } catch (cause) {
      if (revision === this.revision && !this.closed) {
        this.wantsPlayback = false; this.onSnapshot({ ...defaultSnapshot(), songId: id }); this.onError(String(cause));
      }
    }
  }
  private async prepareNext() {
    const revision = this.revision; const id = this.ids[this.ids.indexOf(this.selected) + 1];
    if (!id) { this.next = undefined; this.send({ type: 'next', song: undefined }); return; }
    try {
      const song = await this.resolve(id);
      if (revision !== this.revision || id !== this.ids[this.ids.indexOf(this.selected) + 1] || this.closed) return;
      this.next = song; this.send({ type: 'next', song });
    } catch (cause) { if (revision === this.revision && !this.closed) this.onError(String(cause)); }
  }
  private async advance(id: string) {
    const revision = this.revision;
    try {
      const song = await this.resolve(id);
      if (revision !== this.revision || id !== this.ids[this.ids.indexOf(this.selected) + 1] || this.closed) return;
      this.selected = id; this.song = song; this.next = undefined; this.revision++;
      this.onSong(song);
      this.send({ type: 'load', song, playing: this.wantsPlayback, fromHook: this.settings.hook });
      if (this.wantsPlayback) void this.prepareNext();
    } catch (cause) {
      if (revision === this.revision && !this.closed) { this.wantsPlayback = false; this.send({ type: 'pause' }); this.onError(String(cause)); }
    }
  }
  configure(mode: Mode, mix: Mix[], loop: boolean, hook: boolean, volume: number) {
    this.settings = { mode, mix, loop, hook, volume }; this.send({ type: 'settings', ...this.settings });
  }
  async play(fromHook = false) {
    const revision = this.revision; const id = this.selected;
    if (!id) return;
    this.wantsPlayback = true;
    let song: Song;
    try { [song] = await Promise.all([this.song ?? this.resolve(id), this.ensure()]); }
    catch (cause) { if (revision === this.revision) this.wantsPlayback = false; throw cause; }
    if (revision !== this.revision || !this.wantsPlayback || this.closed) return;
    if (!this.song || fromHook) {
      this.song = song; this.onSong(song);
      this.send({ type: 'load', song, playing: true, fromHook: fromHook || this.settings.hook });
    } else this.send({ type: 'play' });
    void this.prepareNext();
  }
  pause() { this.wantsPlayback = false; this.send({ type: 'pause' }); }
  rewind() { this.send({ type: 'rewind' }); }
  async close() { this.closed = true; this.revision++; this.node?.disconnect(); await this.context?.close(); }
}
