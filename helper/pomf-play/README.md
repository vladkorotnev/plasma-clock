# PoMF Play

Static Svelte 5 player for binary Portable Music Format files. Uses runes, Flowbite Svelte, Tailwind CSS 4, and Vite. All files and audio stay in the browser.

```sh
npm ci
npm run dev
```

The repository's `data/music/*.pomf` files load automatically into the playlist in filename order when the page opens. The first song is selected. Startup downloads only the metadata catalog; bundled song data loads when selected, played, or exported. During playback the next song is prefetched for smooth transitions. Only the current and next bundled songs are retained, while user-added files stay in memory. Additional files can be loaded with **Add files** or by dropping them anywhere on the page. Select, reorder, or remove playlist entries. Playback advances to the next file when the current file ends. **Loop** repeats the current file from its loop marker (or its beginning); **Hook only** starts at the hook and ends at its end marker. Changing Hook only during playback preserves the current position, pending delay, and oscillator state, and changes subsequent hook-end and repeat behavior. While paused, it selects the corresponding starting point. **Play from hook** jumps to the hook without changing Hook only. **Rewind** returns to the beginning and preserves the playing/paused state. Space toggles playback when focus is outside a control.

Master volume works in both modes and in exports, with a short 5 ms ramp during playback to avoid abrupt gain changes. Generator visualizers stay before volume controls. The final Mixed audio row contains the output mode selector and a stereo scope before master volume, displayed at 5× gain (visual scaling only).

The progress bar marks the loop point above the bar and the hook range below it. Without an explicit loop marker, the loop point is the beginning; without a hook end marker, the range extends to EOF.

The Sequence panel follows the current command, with permanent bottom padding so the final line can remain centered. Scrolling manually suspends following; **Follow playhead** restores it. Channel numbers and sample numbers follow the firmware's zero-based numbering; displayed lines are one-based. Piano notes and FREQ_SET command annotations use the same nearest equal-tempered note resolver at A4 = 440 Hz.

## Sound engine

`src/lib/engine.ts` emulates the digital sequence and generator behavior in `src/sound/sequencer.cpp`, `src/sound/generators.cpp`, and `src/sound/waveout.cpp`. No firmware files are modified.

- Six generators: four square oscillators, AY-style LFSR noise, and one RLE sampler with the firmware's default kick sample.
- Generators run at 705,600 bits/second. Integer wavelengths, frequency-triggered phase resets, signed duty divisors, sampler stretch quantization, sampler state changes, and 512-byte firmware block boundaries are preserved.
- Device mode packs and combines generator output with the actual firmware OR/XOR operations, using the same ungated noise signal as HD mode. Removing the firmware noise gate is an intentional exception to bitstream accuracy. It also preserves the `bit = 8 - (s % 8)` truncation, zero-delay behavior, byte padding, silent-block timing, and the sampler stretch counter restarting at each generator call. Per-channel volume and pan have no effect in this mode. A fixed 15 dB gain reduction brings device output close to the default HD mix; exact relative loudness depends on the sequence.
- HD mode uses the individual one-bit generator signals before firmware bit packing. They are summed with independent linear volume and equal-power pan, with headroom for six channels. Both modes use the ungated LFSR bit held for a complete clock cycle, removing the periodic carrier created by the firmware gate; the LFSR taps and off/resume state remain unchanged. The noise clock includes the additional factor of two calibrated against perceived device sound (`value * 2 * 2`); both modes share it. The sampler's XOR flag affects device mixing only.
- Both modes integrate source bits into the browser's audio sample rate and apply a 20 Hz DC blocker. Device accuracy refers to the sequencer's digital bitstream, with the shared ungated noise exception described above; a browser output device does not reproduce the clock's speaker, amplifier, or I2S hardware acoustics. The firmware leaves the initial noise state unspecified; this player initializes it to false for repeatable output.

Delay multiplication uses safe numeric widths, avoiding the firmware's potential signed-integer overflow for long delays.

The engine is organized into focused modules:

- `commands.ts`: command, loop-marker, and sample-mixing enums, opcode validation, and command formatting.
- `pomf.ts`: binary parsing and bounds validation.
- `generators/voice.ts`: common streaming `Voice` interface; `square.ts`, `noise.ts`, and `sampler.ts` provide their respective implementations. `default-sample.ts` holds the kick data, and `index.ts` creates the six-channel voice stack.
- `synthesis/block.ts`: generator blocks, device bit packing, and OR/XOR combination.
- `synthesis/mixer.ts`: stereo channel mixing, DC blocking, master gain ramps, and the pre-fader signal.
- `synthesis/constants.ts`: source rate, block dimensions, channel count, and device gain.
- `engine.ts`: sequence control and host-rate streaming integration.

Playback runs inside an AudioWorklet with an interactive latency hint, retaining only one firmware block (about 5.8 ms) of generated audio. It never renders a song into an AudioBuffer. Status and pre-volume oscilloscope data arrive about 60 times per second. Oscilloscopes show the host-rate integrated generator signal before volume and pan. Commands are virtualized. Generators form a compact horizontal track stack, with controls, readings, piano, and scope aligned across each row. Noise scopes occupy the combined piano/scope area; sampler waveform displays span the full row. Active channel numbers use inverted colors. Recorded sample waveforms display the complete RLE data; scaled waveforms use a fixed 100 ms window that follows the moving playhead so frequency changes visibly stretch the sample.

## Binary files

The loader reads packed 102-byte `POmf` headers, 12-byte chunk headers, 5-byte command rows, and 16-byte ESP32 sample descriptors. It supports `saMP`/`tuNE` and raw-DEFLATE `saZZ`/`tuZZ` chunks, plus the `eof ` sentinel produced by `helper/elf2pomf.py`. Version 1.0 and 1.1 files use this layout. It validates bounds, command channels, sample references, and sample chunk boundaries. Limits are 32 MiB per file and 64 MiB expanded data.

Some older files, including `data/music/wip_yy.pomf`, declare more sample runs than their chunk contains (1690 declared, 1627 present in its third sample). The loader uses the available runs and loops within that data. Extra bytes beyond a smaller declared length are ignored. No missing data or reads outside the sample chunk are used.

## Audio export

Choose WAV or MP3, then **Export** and download the result. A separate Worker runs a fresh instance of the same engine from the beginning to EOF, using the mode, HD mixer settings, and master volume captured when export starts. Playback position, loop and hook settings, and later control changes do not affect conversion. Playback continues during conversion. Cancellation terminates the Worker.

WAV output is stereo 44.1 kHz 16-bit PCM; MP3 output is stereo 44.1 kHz at 192 kbps, encoded with `@breezystack/lamejs` (LGPL-3.0). Export is limited to one hour per file. MP3 is lossy; device-mode WAV represents the resampled one-bit source, not a 705.6 kHz one-bit container.

## Build and GitHub Pages

```sh
npm run check
npm test
npm run build
npm run preview
```

Publish the **contents of `dist/`** to any GitHub Pages directory. `base: './'` makes scripts, styles, the AudioWorklet, and export Worker resolve correctly below a repository path, including `/pomfplay/`. No backend, SPA routing fallback, cross-origin isolation headers, or CDN access is required. Serve over HTTPS (or localhost for development) for AudioWorklet support. Opening `index.html` directly with `file://` does not enable playback.

Every build emits the current `data/music/*.pomf` binaries unchanged into `dist/music/`, plus `music/index.json` listing their metadata in filename order. The catalog and files use relative URLs, so automatic loading works under a GitHub Pages subdirectory. The catalog contains titles, durations, and marker positions, without command or sample data. Binary files are fetched and parsed on demand; simultaneous requests for the same song share one download, and failed requests can be retried. Manual imports made during startup retain their selection. The development server serves the same library directly from the repository.

The repository's existing web publication workflow runs this build and copies the complete output, including the music library, into `webroot/pomfplay/`. No extra workflow copy step is needed; additions and removals in `data/music/` are reflected by the next build.

Browser verification:

```sh
npx playwright install chromium
npm run build
npm run test:browser
```

Tests cover compressed/uncompressed binaries, embedded samples, malformed files, firmware packing, OR/XOR mixing, duty/frequency quantization, noise, hook/loop behavior, pause, zero delays, repository binaries, browser playback, playlist edits and advancement, mixer controls, responsive layout, and whole-file WAV/MP3 conversion. Browser tests serve `dist/` under `/pomfplay/` with a plain static server to exercise GitHub Pages asset resolution.
