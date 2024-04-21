#!/usr/bin/env python3

# this is very jank, do not expect it to work as is
# it worked for some melodies though

from sys import argv
from mido import MidiFile
import freq_note_converter

mid = MidiFile(argv[1])
name = argv[2]

evts = [] # negative: delay in ms, positive: freq in hz

for msg in mid:
    if msg.type == "note_on" or msg.type == "note_off":
        print(msg)
        if msg.time > 0:
            evts.append(int(-msg.time * 1000))
        if msg.type == "note_on" and msg.velocity > 0:
            if len(evts) > 0 and evts[-1] == 0:
                # remove useless silence
                evts[-1] = int(freq_note_converter.from_note_index(msg.note).freq)
            else:
                evts.append(int(freq_note_converter.from_note_index(msg.note).freq))
        else:
            evts.append(0)
        
        
print(evts)

print("static const melody_item_t "+name+"_data[] = {")
i = 0
while i < len(evts) - 1:
    # assert evts[i] < 0
    # assert evts[i+1] >= 0
    print("    {"+str(evts[i])+", "+str(-evts[i+1])+"}, ")
    i+=2
print("};")

print("const melody_sequence_t "+name+" = MELODY_OF("+name+"_data);")