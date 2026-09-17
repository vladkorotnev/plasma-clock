import type { CommandOpcode, SampleMixMode } from './commands';
import { CHANNEL_COUNT } from './synthesis/constants';

export type Mode = 'device' | 'hd';
export interface Command { command: CommandOpcode; channel: number; argument: number; time: number }
export interface Sample { sampleRate: number; rootFrequency: number; mode: SampleMixMode; data: Uint8Array; length: number }
export interface Song {
  id: string; filename: string; title: string; longTitle: string; version: number;
  bytes: number; commands: Command[]; samples: Sample[]; duration: number; loopStart: number; hookStart: number; hookEnd: number;
}
export interface SongInfo extends Omit<Song, 'commands' | 'samples'> {
  commandCount: number; sampleCount: number; loopTime: number; hookStartTime: number; hookEndTime: number;
}
export interface PlaylistEntry extends SongInfo { url?: string; local?: Song }
export interface Mix { volume: number; pan: number }
export interface VoiceStatus {
  active: boolean; frequency: number; duty: number; dutyCycle: number; sample: number; stretch: number; samplePosition: number;
}
export interface Snapshot {
  songId: string; playing: boolean; ended: boolean; line: number; time: number;
  voices: VoiceStatus[]; scopes: Float32Array[]; output: Float32Array[];
}
export const defaultMix = (): Mix[] => Array.from({ length: CHANNEL_COUNT }, () => ({ volume: 1, pan: 0 }));
export const defaultSnapshot = (): Snapshot => ({ songId: '', playing: false, ended: false, line: 0, time: 0,
  voices: Array.from({ length: CHANNEL_COUNT }, () => ({ active: false, frequency: 0, duty: 2, dutyCycle: 0.5, sample: -1, stretch: 1, samplePosition: 0 })), scopes: [], output: [] });
export const formatTime = (seconds: number) => `${Math.floor(seconds / 60).toString().padStart(2, '0')}:${Math.floor(seconds % 60).toString().padStart(2, '0')}`;
