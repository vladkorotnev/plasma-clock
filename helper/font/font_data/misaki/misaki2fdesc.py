#-*- coding: utf8 -*-
import pdb
import json

INNAME="misaki_mincho.bdf"

out_bmp_buf = bytearray()
out_ranges = []

from bdfparser import Font
font = Font(INNAME)

current_range = {"start": 0, "end": 0, "offset": 0}

for glyph in font.iterglyphs(order=1):
    if glyph.cp() != current_range["end"] + 1:
        # Break of range
        out_ranges.append(current_range)
        current_range = {"start": glyph.cp(), "end": glyph.cp(), "offset": len(out_bmp_buf)}
    current_range["end"] = glyph.cp()
    tmp = glyph.draw().bytepad().todata()
    tmp = bytes(map(lambda x: int(x, 2), tmp))
    assert len(tmp) == 8
    out_bmp_buf.extend(tmp)
out_ranges.append(current_range)

with open(INNAME + ".bin", 'wb') as f:
    f.write(out_bmp_buf)

fdesc = {
    "encoding": "utf16",
    "glyph_format": "horizontal",
    "width": 8,
    "height": 8,
    "cursor": 95,
    "invalid": 9670,
    "bitmap_file": INNAME + ".bin",
    "ranges": out_ranges
}

with open(INNAME + ".fdesc", 'w', encoding="utf8") as f:
    json.dump(fdesc, f, ensure_ascii=False, indent=4)

