#!/usr/bin/env python3

import pdb
from sys import argv
from mido import MidiFile
import freq_note_converter

mid = MidiFile(argv[1])
name = argv[2]
long_name = argv[3]

USE_TBL = True
NOTE_TBL = [
    31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 
    62, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117,
    123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233,
    247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466,
    494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 
    988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865,
    1976,2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 
    3951,4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6644, 7040, 7458
]

def getPitch(noteNo):
    global USE_TBL, NOTE_TBL
    if USE_TBL:
        return NOTE_TBL[noteNo]/2
    else:
        return freq_note_converter.from_note_index(noteNo).freq/2

ended = False

class Event():
    def __init__(self, kind, chan, arg):
        self.kind = kind
        self.chan = chan
        self.arg = arg

    def __str__(self):
        return f"    {{{self.kind}, {str(self.chan)}, {str(self.arg)}}},"

class Comment():
    def __init__(self, s):
        self.content = s
        self.kind = "REM"

    def __str__(self):
        return f"    /* {self.content} */"

evts = []

def prev_note_off_event(chan):
    for i in range(1,len(evts)+1):
        e = evts[-i]
        if e.kind == "FREQ_SET" and e.arg == 0 and e.chan == chan:
            return e
        elif e.kind == "DELAY":
            return None
    return None

for msg in mid:
    print(msg)
    if msg.time >= 0.01:
            evts.append(Event("DELAY", 0, int(msg.time * 1000)))
    if msg.type == "note_on" or msg.type == "note_off":
        if msg.type == "note_on" and msg.velocity > 0:
            existing_evt = prev_note_off_event(msg.channel)
            if existing_evt is not None:
                existing_evt.arg = int(getPitch(msg.note))
            else:
                evts.append(Event("FREQ_SET", msg.channel, int(getPitch(msg.note))))
        else:
            # note off
            evts.append(Event("FREQ_SET", msg.channel, 0))
    elif msg.type == "end_of_track":
        if ended:
            raise Exception("WTF, already ended")
        ended = True
    elif msg.type == "marker":
        evts.append(Comment(msg.text))
        if msg.text == "LOOP":
            evts.append(Event("LOOP_POINT_SET", 0, "LOOP_POINT_TYPE_LOOP"))
        elif msg.text == "HOOK":
            evts.append(Event("LOOP_POINT_SET", 0, "LOOP_POINT_TYPE_HOOK_START"))
        elif msg.text == "HOOKEND":
            evts.append(Event("LOOP_POINT_SET", 0, "LOOP_POINT_TYPE_HOOK_END"))
        elif msg.text.startswith("SAMPLE="):
            evts.append(Event("SAMPLE_LOAD", 5, "(int) &" + msg.text[len("SAMPLE=")::]))
    elif msg.type == "control_change":
        if msg.control == 2:
            evts.append(Event("DUTY_SET", msg.channel, int(msg.value)))

print('#include <sound/pomf.h>')     
print('extern "C" const POMFHeader POMF_HEAD = {')
print('    POMF_MAGIC_FILE,')
print('    POMF_CURVER,')
print('    "'+name+'",')
print('    "'+long_name+'",')
print('};')
print('')
print('extern "C" const melody_item_t POMF_TUNE[] = {')
for e in evts:
    print(str(e))
print("};")
print('')