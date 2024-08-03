#!/usr/bin/env python3

import pdb
from sys import argv
from mido import MidiFile
import freq_note_converter

mid = MidiFile(argv[1])
name = argv[2]

ended = False

class Event():
    def __init__(self, kind, chan, arg):
        self.kind = kind
        self.chan = chan
        self.arg = arg

    def __str__(self):
        return f"    {{{self.kind}, {str(self.chan)}, {str(int(self.arg))}}},"

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
    if msg.type == "note_on" or msg.type == "note_off":
        if msg.time > 0.005:
            evts.append(Event("DELAY", 0, msg.time * 1000))
        if msg.type == "note_on" and msg.velocity > 0:
            existing_evt = prev_note_off_event(msg.channel)
            if existing_evt is not None:
                existing_evt.arg = freq_note_converter.from_note_index(msg.note).freq
            else:
                evts.append(Event("FREQ_SET", msg.channel, freq_note_converter.from_note_index(msg.note).freq))
        else:
            # note off
            evts.append(Event("FREQ_SET", msg.channel, 0))
    elif msg.type == "end_of_track":
        if ended:
            raise Exception("WTF, already ended")
        ended = True
        evts.append(Event("DELAY", 0, msg.time * 1000))
    elif msg.type == "marker":
        if msg.time > 0.005:
            evts.append(Event("DELAY", 0, msg.time * 1000))
        evts.append(Comment(msg.text))
        if msg.text == "LOOP":
            evts.append(Event("LOOP_POINT_SET", 0, 0))
        

print("static const melody_item_t "+name+"_data[] = {")
for e in evts:
    print(str(e))
print("};")

print("const melody_sequence_t "+name+" = MELODY_OF("+name+"_data);")