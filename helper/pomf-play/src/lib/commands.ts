import type { Command } from './types';
import { frequencyToNote } from './notes';

/** Numeric values are the low nibble of a PoMF command byte. */
export enum CommandOpcode {
  FREQ_SET = 0,
  DUTY_SET = 1,
  DELAY = 2,
  LOOP_POINT_SET = 3,
  SAMPLE_LOAD = 4,
}

export enum LoopMarker { LOOP = 0, HOOK_START = 1, HOOK_END = 2 }
export enum SampleMixMode { ADD = 0, XOR = 1 }

export function isCommandOpcode(value: number): value is CommandOpcode {
  switch (value) {
    case CommandOpcode.FREQ_SET:
    case CommandOpcode.DUTY_SET:
    case CommandOpcode.DELAY:
    case CommandOpcode.LOOP_POINT_SET:
    case CommandOpcode.SAMPLE_LOAD: return true;
    default: return false;
  }
}

export const commandUsesChannel = (command: CommandOpcode) => command === CommandOpcode.FREQ_SET
  || command === CommandOpcode.DUTY_SET || command === CommandOpcode.SAMPLE_LOAD;

const names: Record<CommandOpcode, string> = {
  [CommandOpcode.FREQ_SET]: 'FREQ_SET',
  [CommandOpcode.DUTY_SET]: 'DUTY_SET',
  [CommandOpcode.DELAY]: 'DELAY',
  [CommandOpcode.LOOP_POINT_SET]: 'LOOP_POINT',
  [CommandOpcode.SAMPLE_LOAD]: 'SAMPLE_LOAD',
};
const markers: Record<LoopMarker, string> = {
  [LoopMarker.LOOP]: 'LOOP',
  [LoopMarker.HOOK_START]: 'HOOK START',
  [LoopMarker.HOOK_END]: 'HOOK END',
};
export const commandName = (row: Command) => names[row.command];
export function commandValue(row: Command): string {
  switch (row.command) {
    case CommandOpcode.FREQ_SET: return row.argument ? `${row.argument.toLocaleString()} Hz · ${frequencyToNote(row.argument)!.name}` : 'OFF';
    case CommandOpcode.DUTY_SET: return `1/${Math.abs(row.argument) < 2 ? 2 : Math.abs(row.argument)}${row.argument < -1 ? ' inverted' : ''}`;
    case CommandOpcode.DELAY: return `${row.argument.toLocaleString()} ms`;
    case CommandOpcode.LOOP_POINT_SET: return markers[row.argument as LoopMarker];
    case CommandOpcode.SAMPLE_LOAD: return `Sample ${row.argument}`;
  }
}
