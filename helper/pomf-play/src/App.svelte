<script lang="ts">
  import { onMount } from 'svelte';
  import { Button, Toggle, Select } from 'flowbite-svelte';
  import { AudioPlayer } from './lib/audio';
  import { parsePomf } from './lib/pomf';
  import { loadMusicLibrary, SongLoader, songInfo } from './lib/library';
  import { CommandOpcode, commandName, commandValue, commandUsesChannel } from './lib/commands';
  import { defaultMix, defaultSnapshot, formatTime, type Mix, type Mode, type Song, type PlaylistEntry } from './lib/types';
  import LoadingIndicator from './components/LoadingIndicator.svelte';
  import Icon from './components/Icon.svelte';
  import Track from './components/Track.svelte';
  import MixedTrack from './components/MixedTrack.svelte';

  let songs = $state.raw<PlaylistEntry[]>([]);
  let selectedId = $state('');
  let loadingIds = $state.raw<Set<string>>(new Set());
  let catalogLoading = $state(true);
  let selectedLoading = $derived(loadingIds.has(selectedId));
  let waitingForNext = $derived(snapshot.ended && snapshot.playing && loadingIds.has(songs[selectedIndex + 1]?.id));
  let info = $derived(songs.find(song => song.id === selectedId));
  let current = $state.raw<Song | undefined>();
  let loopTime = $derived(info?.loopTime ?? 0);
  let hookStartTime = $derived(info?.hookStartTime ?? 0);
  let hookEndTime = $derived(info?.hookEndTime ?? 0);
  let selectedIndex = $derived(songs.findIndex(song => song.id === selectedId));
  let snapshot = $state.raw(defaultSnapshot());
  let mode = $state<Mode>('device');
  let volume = $state(1);
  let mix = $state<Mix[]>(defaultMix());
  let loop = $state(false); let hook = $state(false);
  let follow = $state(true); let error = $state(''); let importing = $state(false);
  let exporting = $state(false); let exportProgress = $state(0); let format = $state('wav');
  let download = $state<{ url: string; name: string } | null>(null);
  let input: HTMLInputElement; let commandList: HTMLDivElement;
  let scrollTop = $state(0); let listHeight = $state(600);
  let bottomPadding = $derived(Math.max(0, listHeight / 2 - 16));
  let firstRow = $derived(Math.min(Math.max(0, (current?.commands.length ?? 1) - 1), Math.max(0, Math.floor(scrollTop / 32) - 6)));
  let lastRow = $derived(firstRow + Math.ceil(listHeight / 32) + 14);
  let visibleRows = $derived(current?.commands.slice(firstRow, lastRow) ?? []);
  let player: AudioPlayer; let loader: SongLoader; let exportWorker: Worker | undefined; let exportRevision = 0;
  const formatOptions = [{ value: 'wav', name: 'WAV · 16-bit PCM' }, { value: 'mp3', name: 'MP3 · 320 kbps' }];

  onMount(() => {
    const controller = new AbortController();
    loader = new SongLoader(controller.signal, ids => loadingIds = new Set(ids));
    player = new AudioPlayer(id => {
      const entry = songs.find(song => song.id === id);
      if (!entry) return Promise.reject(new Error('Song was removed from the playlist.'));
      return loader.load(entry);
    }, song => {
      current = song;
      if (selectedId !== song.id) { selectedId = song.id; snapshot = defaultSnapshot(); follow = true; scrollTop = 0; retainSongs(); }
    }, status => {
      if (status.songId && !songs.some(song => song.id === status.songId)) return;
      snapshot = status;
      if (status.songId && songs.some(song => song.id === status.songId)) selectedId = status.songId;
    }, message => error = message);
    void loadMusicLibrary(controller.signal).then(library => {
      if (controller.signal.aborted) return;
      appendSongs(library);
    }).catch(cause => {
      if (!controller.signal.aborted) error = [error, cause instanceof Error ? cause.message : String(cause)].filter(Boolean).join('\n');
    }).finally(() => { catalogLoading = false; });
    return () => { controller.abort(); void player.close(); exportWorker?.terminate(); if (download) URL.revokeObjectURL(download.url); };
  });
  $effect(() => { player?.configure(mode, mix.map(value => ({ ...value })), loop, hook, volume); });
  $effect(() => {
    if (follow && commandList && current) {
      const target = Math.max(0, snapshot.line * 32 + 16 - listHeight / 2);
      commandList.scrollTop = target;
      scrollTop = commandList.scrollTop;
    }
  });
  async function importFiles(files: FileList | File[] | null) {
    if (!files || importing) return;
    importing = true; error = ''; const loaded: Song[] = []; const errors: string[] = [];
    for (const file of Array.from(files)) {
      try { loaded.push(parsePomf(await file.arrayBuffer(), file.name)); }
      catch (cause) { errors.push(`${file.name}: ${cause instanceof Error ? cause.message : String(cause)}`); }
    }
    appendSongs(loaded.map(song => ({ ...songInfo(song), local: song })));
    error = errors.join('\n'); importing = false; if (input) input.value = '';
  }
  function appendSongs(loaded: PlaylistEntry[]) {
    if (!loaded.length) return;
    songs = [...songs, ...loaded]; retainSongs(); player.playlist(songs.map(song => song.id));
    if (!selectedId) selectSong(loaded[0].id, false, Boolean(loaded[0].local));
  }
  function retainSongs() {
    const index = songs.findIndex(song => song.id === selectedId);
    loader.retain(songs.slice(Math.max(0, index), Math.max(0, index) + 2).map(song => song.id));
  }
  function selectSong(id: string, playing = false, load = true) {
    selectedId = id; current = undefined; snapshot = { ...defaultSnapshot(), playing }; follow = true; scrollTop = 0;
    retainSongs(); void player.select(id, playing, load);
  }
  async function play(fromHook = false) {
    if (!info) return;
    error = ''; const id = selectedId; snapshot = { ...snapshot, playing: true };
    try { await player.play(fromHook); } catch (cause) {
      if (selectedId === id) { snapshot = { ...snapshot, playing: false }; error = cause instanceof Error ? cause.message : String(cause); }
    }
  }
  function pause() { snapshot = { ...snapshot, playing: false }; player.pause(); }
  function move(index: number, offset: number) {
    const next = [...songs]; [next[index], next[index + offset]] = [next[index + offset], next[index]];
    songs = next; retainSongs(); player.playlist(songs.map(song => song.id));
  }
  function remove(index: number) {
    const removed = songs[index]; const playing = snapshot.playing;
    songs = songs.filter((_, i) => i !== index); retainSongs(); player.playlist(songs.map(song => song.id));
    if (removed.id === selectedId) {
      pause();
      const next = songs[Math.min(index, songs.length - 1)];
      if (next) selectSong(next.id, playing);
      else { selectedId = ''; current = undefined; snapshot = defaultSnapshot(); void player.select(''); }
    }
  }
  function changeMix(channel: number, field: 'volume' | 'pan', value: number) { mix[channel][field] = value; }
  async function startExport() {
    if (!info || exporting) return;
    const entry = info; const revision = ++exportRevision;
    const settings = { mode, mix: mix.map(value => ({ ...value })), volume, format };
    if (download) URL.revokeObjectURL(download.url); download = null;
    const name = `${entry.filename.replace(/\.pomf$/i, '')}-${mode}.${format}`;
    exporting = true; exportProgress = 0; error = '';
    let song: Song;
    try { song = await loader.load(entry); }
    catch (cause) { if (revision === exportRevision) { error = String(cause); exporting = false; } return; }
    if (revision !== exportRevision) return;
    if (selectedId === song.id) current = song;
    exportWorker = new Worker(new URL('./lib/export.worker.ts', import.meta.url), { type: 'module' });
    exportWorker.onmessage = ({ data }) => {
      if (data.type === 'progress') exportProgress = data.progress;
      if (data.type === 'done') { download = { url: URL.createObjectURL(data.blob), name }; exporting = false; exportWorker?.terminate(); exportWorker = undefined; }
      if (data.type === 'error') { error = data.message; cancelExport(); }
    };
    exportWorker.onerror = event => { error = event.message || 'Audio export failed.'; cancelExport(); };
    exportWorker.postMessage({ song, ...settings });
  }
  function cancelExport() { exportRevision++; exportWorker?.terminate(); exportWorker = undefined; exporting = false; }
  function keyboard(event: KeyboardEvent) {
    if (event.code !== 'Space' || event.target instanceof HTMLInputElement || event.target instanceof HTMLSelectElement || event.target instanceof HTMLButtonElement || event.target instanceof HTMLAnchorElement) return;
    event.preventDefault(); if (snapshot.playing) pause(); else void play();
  }
  function dragFiles(event: DragEvent) {
    if (!event.dataTransfer?.types.includes('Files')) return;
    event.preventDefault(); event.dataTransfer.dropEffect = 'copy';
  }
  function dropFiles(event: DragEvent) {
    if (!event.dataTransfer?.types.includes('Files')) return;
    event.preventDefault(); void importFiles(event.dataTransfer.files);
  }
</script>

<svelte:window onkeydown={keyboard} ondragover={dragFiles} ondrop={dropFiles} />
<div class="app-shell">
  {#if error}<div class="error-bar" role="alert"><span>{error}</span><button class="icon-button" onclick={() => error = ''} aria-label="Dismiss error"><Icon name="close" /></button></div>{/if}
  <main class="workspace">
    <aside class="left-panel">
      <section class="transport panel">
        <div class="section-heading"><h2>Now playing</h2>{#if selectedLoading || waitingForNext}<LoadingIndicator label={waitingForNext ? 'Loading next song…' : 'Loading song…'} text="Loading…" />{/if}</div>
        <div class="song-info"><h3>{info?.title ?? 'No file selected'}</h3>{#if info?.longTitle}<p>{info.longTitle}</p>{/if}<span class="filename mono">{info?.filename ?? '—'}</span></div>
        <div class="position"><strong class="mono">{formatTime(snapshot.time)}</strong><span class="mono">/ {formatTime(info?.duration ?? 0)}</span></div>
        <div class="position-bar">
          <div style={`width:${Math.min(100, snapshot.time / (info?.duration || 1) * 100)}%`}></div>
          {#if info}
            <span class="loop-marker" style={`left:${loopTime / info.duration * 100}%`} role="img" aria-label="Loop point" title="Loop point"></span>
            {#if info.hookStart >= 0}<span class="hook-range" style={`left:${hookStartTime / info.duration * 100}%;width:${Math.max(0, hookEndTime - hookStartTime) / info.duration * 100}%`} role="img" aria-label="Hook range" title="Hook range"></span>{/if}
          {/if}
        </div>
        <dl class="metadata"><div><dt>Line</dt><dd class="mono">{info ? snapshot.line + 1 : 0} <span>/ {info?.commandCount ?? 0}</span></dd></div><div><dt>Format</dt><dd class="mono">{info ? `PoMF ${info.version >> 8}.${info.version & 255}` : '—'}</dd></div><div><dt>Samples</dt><dd class="mono">{info?.sampleCount ?? 0}</dd></div><div><dt>Hook</dt><dd class="mono">{info && info.hookStart >= 0 ? `L${info.hookStart + 1}${info.hookEnd >= 0 ? `–${info.hookEnd + 1}` : ''}` : '—'}</dd></div></dl>
        <div class="primary-controls"><Button class="play-button" disabled={!info} onclick={() => snapshot.playing ? pause() : play()}><Icon name={snapshot.playing ? 'pause' : 'play'} />{snapshot.playing ? 'Pause' : 'Play'}</Button><Button color="alternative" class="rewind-button" disabled={!current} onclick={() => player.rewind()} title="Rewind"><Icon name="rewind" /></Button></div>
        <Button color="alternative" class="hook-button" disabled={!info || info.hookStart < 0} onclick={() => play(true)}><Icon name="hook" />Play from hook</Button>
        <div class="playback-options"><Toggle bind:checked={loop} size="small">Loop</Toggle><Toggle bind:checked={hook} size="small" disabled={!info || info.hookStart < 0}>Hook only</Toggle></div>
        <label class="master-volume">Volume <input type="range" min="0" max="1" step="0.01" bind:value={volume} aria-label="Master volume" /><output class="mono">{Math.round(volume * 100)}%</output></label>
      </section>
      <section class="playlist panel">
        <div class="section-heading"><h2>Playlist <span class="count">{songs.length}</span></h2><div class="playlist-navigation"><button class="icon-button" disabled={selectedIndex <= 0} onclick={() => selectSong(songs[selectedIndex - 1].id, snapshot.playing)} aria-label="Previous file"><Icon name="previous" size={16} /></button><button class="icon-button" disabled={selectedIndex < 0 || selectedIndex === songs.length - 1} onclick={() => selectSong(songs[selectedIndex + 1].id, snapshot.playing)} aria-label="Next file"><Icon name="next" size={16} /></button></div></div>
        <input bind:this={input} type="file" accept=".pomf" multiple onchange={e => importFiles(e.currentTarget.files)} class="file-input" aria-label="Load PoMF files" />
        <Button color="alternative" class="add-files" onclick={() => input.click()} disabled={importing}><Icon name="plus" />{importing ? 'Loading…' : 'Add files'}</Button>
        <div class="playlist-list" role="region" aria-label="Playlist">
          {#each songs as song, index (song.id)}
            <div class="playlist-item" aria-busy={loadingIds.has(song.id)} class:selected={song.id === selectedId}>
              <button class="playlist-select" onclick={() => selectSong(song.id, snapshot.playing)} aria-current={song.id === selectedId ? 'true' : undefined}><span class="playlist-index mono">{#if loadingIds.has(song.id)}<LoadingIndicator label={`Loading ${song.filename}`} />{:else}{(index + 1).toString().padStart(2, '0')}{/if}</span><span class="playlist-title"><strong>{song.title}</strong><small class="mono">{formatTime(song.duration)} · {song.filename}</small></span></button>
              <div class="playlist-actions"><button class="icon-button" disabled={index === 0} onclick={() => move(index, -1)} aria-label={`Move ${song.title} up`}><Icon name="up" size={14} /></button><button class="icon-button" disabled={index === songs.length - 1} onclick={() => move(index, 1)} aria-label={`Move ${song.title} down`}><Icon name="down" size={14} /></button><button class="icon-button" onclick={() => remove(index)} aria-label={`Remove ${song.title}`}><Icon name="close" size={14} /></button></div>
            </div>
          {:else}{#if catalogLoading}<div class="playlist-empty"><LoadingIndicator label="Loading playlist…" text="Loading playlist…" /></div>{:else}<div class="playlist-empty"><Icon name="folder" size={30} /><p>Drop PoMF files here</p></div>{/if}{/each}
        </div>
      </section>
      <section class="export-panel panel"><div class="section-heading"><h2>Export audio</h2></div><Select items={formatOptions} bind:value={format} aria-label="Export format" class="format-select" /><Button color="alternative" class="export-button" disabled={!info || exporting} onclick={startExport}><Icon name="download" />Export {format.toUpperCase()}</Button>
        {#if exporting}<div class="export-progress"><progress max="1" value={exportProgress}></progress><span class="mono">{Math.round(exportProgress * 100)}%</span><button class="icon-button" onclick={cancelExport} aria-label="Cancel export"><Icon name="close" size={14} /></button></div>{/if}
        {#if download}<a class="download-link" href={download.url} download={download.name}>Download {download.name}</a>{/if}
      </section>
    </aside>
    <section class="sequence-panel panel">
      <div class="section-heading"><h2>Sequence</h2><button class="follow-button" class:following={follow} aria-pressed={follow} onclick={() => follow = !follow}>Follow playhead</button></div>
      <div class="command-header command-grid"><span>Line</span><span>Time</span><span>Ch</span><span>Command</span><span>Value</span></div>
      <!-- svelte-ignore a11y_no_noninteractive_tabindex, a11y_no_noninteractive_element_interactions (focus and key events support native scrolling in this labelled region) -->
      <div class="command-list" bind:this={commandList} bind:clientHeight={listHeight} onscroll={e => scrollTop = e.currentTarget.scrollTop} onwheel={() => follow = false} ontouchmove={() => follow = false} onpointerdown={() => follow = false} onkeydown={() => follow = false} tabindex="0" role="region" aria-label="PoMF commands">
        {#if current}<div class="command-content" style={`height:${current.commands.length * 32 + bottomPadding}px;position:relative`}><div style={`position:absolute;top:${firstRow * 32}px;left:0;right:0`}>
          {#each visibleRows as row, index (firstRow + index)}{@const line = firstRow + index}<div class="command-row command-grid" class:current-line={line === snapshot.line} class:past-line={line < snapshot.line} class:marker-row={row.command === CommandOpcode.LOOP_POINT_SET}><span class="line-number mono">{line + 1}</span><span class="command-time mono">{formatTime(row.time)}<small>.{Math.round((row.time % 1) * 1000).toString().padStart(3, '0')}</small></span><span class="command-channel mono">{commandUsesChannel(row.command) ? row.channel.toString().padStart(2, '0') : '—'}</span><span class="command-name mono">{commandName(row)}</span><span class="command-value mono" title={commandValue(row)}>{commandValue(row)}</span></div>{/each}
        </div></div>{:else if selectedLoading}<div class="sequence-empty"><LoadingIndicator label="Loading sequence…" text="Loading sequence…" /></div>{:else}<div class="sequence-empty"><Icon name="wave" size={44} /><p>No sequence loaded</p></div>{/if}
      </div>
      <div class="sequence-footer"><span>{current ? `${current.commands.length.toLocaleString()} commands` : '0 commands'}</span><span>{current ? `${(current.bytes / 1024).toFixed(1)} KiB` : '—'}</span></div>
    </section>
    <aside class="tracks-panel" aria-label="Audio channels"><div class="tracks-list">{#each snapshot.voices as voice, channel}<Track {channel} {voice} scope={snapshot.scopes[channel]} mix={mix[channel]} {mode} samples={current?.samples ?? []} onchange={changeMix} />{/each}<MixedTrack bind:mode output={snapshot.output} playing={snapshot.playing} /></div></aside>
  </main>
</div>
