#!/usr/bin/env python3

# this is very jank, do not expect it to work as is
# it worked for some melodies though

from sys import argv
from mido import MidiFile
import freq_note_converter

mid = MidiFile(argv[1])
name = argv[2]

last_time = 0
evts = [] # of (freq in hz or 0, delay in ms)
ended = False

for msg in mid:
    if msg.type == "note_on" or msg.type == "note_off":
        print(msg)
        if msg.time > 0 and len(evts) > 0 and evts[-1][1] == 0:
            evts[-1][1] = int(msg.time * 1000)
        if msg.type == "note_on" and msg.velocity > 0:
            evts.append([int(freq_note_converter.from_note_index(msg.note).freq), 0, ""])
        else:
            # note off
            evts.append([0, 0, ""])
    elif msg.type == "end_of_track":
        print(msg)
        if ended:
            raise Exception("WTF, already ended")
        ended = True
        if evts[-1][0] == 0:
            # pause exists, just extend it
            evts[-1][1] = int(msg.time * 1000)
        else:
            evts.append([0, int(msg.time*1000), ""])
    elif msg.type == "marker":
        evts[-1][2] = msg.text
        
        
print(evts)

print("static const melody_item_t "+name+"_data[] = {")
i = 0
while i < len(evts):
    if evts[i][0] != 0 or evts[i][1] != 0:
        print("    {"+str(evts[i][0])+", "+str(evts[i][1])+"}, ")
    if evts[i][2] != "":
        print("    ")
        print("    // " + evts[i][2])
    i+=1
print("};")

print("const melody_sequence_t "+name+" = MELODY_OF("+name+"_data);")