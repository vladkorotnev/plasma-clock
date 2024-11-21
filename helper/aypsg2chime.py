#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, sys, pdb, struct, math
from argparse import ArgumentParser

# PSG uses 20ms delays or multiples of 80ms delays
PSG_CLOCK = 1750000
MS_IN_FF_COMMAND = 20
MS_IN_MULTIPLE_COMMAND = 80

parser = ArgumentParser(prog='AYPSG2Chime', description="Converts PSG of AY-3-8910 to chime source for PIS-OSI")
parser.add_argument('infile')
parser.add_argument('name')
parser.add_argument('long_name')
args = parser.parse_args()

INFILE=open(args.infile, 'rb').read()
if INFILE[0:4] != b"PSG\x1A":
    print("Not PSG file")
    os.exit(1)

if INFILE[4] != 0x0:
    print("New PSG format, todo: read freq, abort!")
    os.exit(1)

start_pos = 4
while INFILE[start_pos] != 0xFF:
    start_pos += 1

INDAT=INFILE[start_pos::]
pos = 0
timestamp = 0
eos = False
evts = []

NOISE_CHAN = 4 
AMPLITUDE_TO_DUTY=[
    16, # 0, -> map to note off
    13, # 1,
    12, # 2,
    12, # 3,
    11, # 4,
    11, # 5,
    10, # 6,
    10, # 7,
    10, # 8,
    8, # 9,
    8, # A,
    8, # B,
    8, # C,
    6, # D,
    6, # E,
    4, # F,
]

# A, B, C
tone_state=[True, True, True]
tone_ampli_state=[True, True, True]
noise_ampli_state=[True, True, True]
tone_raw_val = [0, 0, 0]
tone_val = [0, 0, 0]
noise_state=[False, False, False]
noise_val = 0

def prev_note_event(chan):
    for i in range(1,len(evts)+1):
        e = evts[-i]
        if e.kind == "FREQ_SET" and e.chan == chan:
            return e
        elif e.kind == "DELAY":
            return None
    return None

class Event():
    def __init__(self, kind, chan, arg):
        self.kind = kind
        self.chan = chan
        self.arg = arg

    def __str__(self):
        return f"    {{{self.kind}, {str(self.chan)}, {str(self.arg)}}},"

evts.append(Event("DUTY_SET", 0, 8))
evts.append(Event("DUTY_SET", 1, 8))
evts.append(Event("DUTY_SET", 2, 8))

VOL_THRESH = 0

while not eos and pos < len(INDAT):
    cmd = INDAT[pos]
    if cmd == 0xFD:
        eos = True
    elif cmd == 0xFF:
        # delay TICKS_IN_FF_COMMAND
        if len(evts) > 0 and evts[-1].kind == "DELAY":
            evts[-1].arg += MS_IN_FF_COMMAND
        else:
            evts.append(Event("DELAY", 0, MS_IN_FF_COMMAND))
    elif cmd == 0xFE:
        pos += 1
        count = INDAT[pos]
        # delay TICKS_IN_MULTIPLE_COMMAND * count
        if len(evts) > 0 and evts[-1].kind == "DELAY":
            evts[-1].arg += MS_IN_MULTIPLE_COMMAND * count
        else:
            evts.append(Event("DELAY", 0, MS_IN_MULTIPLE_COMMAND * count))
    else:
        regi = cmd
        pos += 1
        valu = INDAT[pos]
        # write `valu` to AY `regi`
        # R10,11,12: AMPLITUDE CONTROL
        if regi >= 10 and regi <= 12:
            chan = regi - 10
            if valu & 0x10 == 0:
                # Envelope mode, we don't have envelope
                valu = 10
            else:
                valu = valu & 0xF
                valu = 0xF - valu
            if valu <= VOL_THRESH:
                if tone_state[chan] and tone_ampli_state[chan]:
                    evts.append(Event("FREQ_SET", chan, 0))
                    tone_ampli_state[chan] = False
                if noise_state[chan] and noise_ampli_state[chan]:
                    evts.append(Event("FREQ_SET", NOISE_CHAN, 0))
                    noise_ampli_state[chan] = False
            elif valu > VOL_THRESH:
                if tone_val[chan] > 0 and tone_state[chan] and not tone_ampli_state[chan]:
                    evts.append(Event("FREQ_SET", chan, tone_val[chan]))
                    tone_ampli_state[chan] = True
                if noise_state[chan] and not noise_ampli_state[chan] and noise_val > 0:
                    evts.append(Event("FREQ_SET", NOISE_CHAN, noise_val))
                    noise_ampli_state[chan] = True
            if tone_state[chan] and tone_ampli_state[chan] and valu > 0:
                evts.append(Event("DUTY_SET", chan, AMPLITUDE_TO_DUTY[valu]))
        # R6: NOISE CONTROL
        elif regi == 6:
            if noise_state[0] or noise_state[1] or noise_state[2]:
                valu = valu & 0x1F
                freq = PSG_CLOCK // 32
                if valu == 0:
                    freq //= 8
                else:
                    freq //= valu
                    freq //= 8
                noise_val = freq
                evts.append(Event("FREQ_SET", NOISE_CHAN, freq))
        # R7: IO !ENABLE
        elif regi == 7:
            old_tone_state = tone_state.copy()
            old_noise_state = (noise_state[0] and noise_ampli_state[0]) or (noise_state[1] and noise_ampli_state[1]) or (noise_state[2] and noise_ampli_state[2])
            for i in range(0,3):
                tone_state[i] = ((valu & (1 << i)) == 0)
                if tone_state[i] != old_tone_state[i]:
                    if tone_state[i] and tone_val[i] != 0:
                        evts.append(Event("FREQ_SET", i, tone_val[i]))
                    elif not tone_state[i]:
                        evts.append(Event("FREQ_SET", i, 0))
                noise_state[i] = (valu & (1 << (3 + i))) == 0
            new_noise_state =  (noise_state[0] and noise_ampli_state[0]) or (noise_state[1] and noise_ampli_state[1]) or (noise_state[2] and noise_ampli_state[2])
            if new_noise_state and not old_noise_state and noise_val != 0:
                evts.append(Event("FREQ_SET", NOISE_CHAN, noise_val))
            elif old_noise_state and not new_noise_state:
                evts.append(Event("FREQ_SET", NOISE_CHAN, 0))
        # R0-R5: TONE CONTROL
        elif regi >= 0 and regi <= 5:
            chan = regi // 2
            is_fine = (regi % 2) == 0
            old_raw_val = tone_raw_val[chan]
            if is_fine:
                tone_raw_val[chan] &= 0xF00
                tone_raw_val[chan] |= valu
            else:
                tone_raw_val[chan] &= 0xFF
                tone_raw_val[chan] |= ((valu & 0xF) << 8)
            tone_val[chan] = PSG_CLOCK // 16
            if tone_raw_val[chan] == 0:
                tone_val[chan] //= 0x1000
            else:
                tone_val[chan] //= tone_raw_val[chan]
            if old_raw_val != tone_raw_val[chan] and tone_state[chan] and tone_ampli_state[chan]:
                existing_evt = prev_note_event(chan)
                if existing_evt is not None:
                    existing_evt.arg = tone_val[chan]
                else:
                    evts.append(Event("FREQ_SET", chan, tone_val[chan]))

    pos += 1

print('#include <sound/pomf.h>')     
print('extern "C" const POMFHeader POMF_HEAD = {')
print('    POMF_MAGIC_FILE,')
print('    POMF_CURVER,')
print('    "'+args.name+'",')
print('    "'+args.long_name+'",')
print('};')
print('')
print('extern "C" const melody_item_t POMF_TUNE[] = {')
for e in evts:
    if e.kind == "DELAY" and e.arg > 1000:
        x = e.arg
        e.arg = 1000
        while x > 1000:
            print(str(e))
            x -= 1000
        e.arg = x
    print(str(e))
print("};")
print('')