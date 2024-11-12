#-*- coding: utf8 -*-
import pdb
import json

INNAME="jiskan16.bdf"

out_bmp_buf = bytearray()
out_ranges = []

from bdfparser import Font
font = Font(INNAME)

current_range = {"start": 0, "end": 0, "offset": 0}

# Read the second and third column of JIS0208.TXT into a dictionary
jis0208 = {}
with open("JIS0208.TXT", "r", encoding="utf8") as f:
    for line in f:
        line = line.strip()
        if line == "" or line.startswith("#"):
            continue
        _, b, c, _ = line.split('	')
        jis0208[int(b, 16)] = int(c, 16)

def shift_jis2unicode(cp):
    if cp in jis0208:
        return jis0208[cp]
    else:
        return 0

for glyph in font.iterglyphs(order=1):
    realcp = shift_jis2unicode(glyph.cp())
    if realcp != current_range["end"] + 1:
        # Break of range
        out_ranges.append(current_range)
        current_range = {"start": realcp, "end": realcp, "offset": len(out_bmp_buf)}
    current_range["end"] = realcp
    tmp = glyph.draw(2).bytepad().todata()
    tmp = bytes(int(y, 2) for x in tmp for y in [x[0:8], x[8:16]])
    assert len(tmp) == 32
    out_bmp_buf.extend(tmp)
out_ranges.append(current_range)

with open(INNAME + ".bin", 'wb') as f:
    f.write(out_bmp_buf)

fdesc = {
    "encoding": "utf16",
    "glyph_format": "horizontal",
    "width": 16,
    "height": 16,
    "cursor": 95,
    "invalid": 9670,
    "bitmap_file": INNAME + ".bin",
    "ranges": out_ranges
}

with open(INNAME + ".fdesc", 'w', encoding="utf8") as f:
    json.dump(fdesc, f, ensure_ascii=False, indent=4)

print(len(out_ranges))