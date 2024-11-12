#-*- coding: utf-8 -*-
import json
import zlib
import pdb
from sys import argv

FS_BLKSZ = 4096
INNAME=argv[1]
OUTNAME=argv[2]

def compress_bytes(data, level = 5):
    assert(zlib.MAX_WBITS == 15)
    compress = zlib.compressobj(
        level,
        zlib.DEFLATED,
        -zlib.MAX_WBITS,
        zlib.DEF_MEM_LEVEL,
        0
    )
    rslt = compress.compress(data)
    rslt += compress.flush()
    return rslt

with open(INNAME, 'r', encoding="utf8") as f:
    fdesc = json.load(f)

# Calculate character size in bytes, accounting for horizontal or native (vertical) pixel layout
if fdesc['glyph_format'] == 'horizontal':
    char_size = max(1, fdesc['width'] // 8) * fdesc['height']
elif fdesc['glyph_format'] == 'vertical':
    char_size = max(1, fdesc['height'] // 8) * fdesc['width']
else:
    raise ValueError("Unknown glyph format " + fdesc['glyph_format'])

print(char_size)
buffer = bytearray()
with open(fdesc['bitmap_file'], 'rb') as bitmap_file:
    buffer.extend(bitmap_file.read())

# Sort ranges by start value
ranges = sorted(fdesc['ranges'], key=lambda x: x["start"])

for i in range(len(ranges)):
    start, end = ranges[i]["start"], ranges[i]["end"]
    offset = ranges[i]["offset"]

    # Parse the ranges' offset value. If it is a number, treat it as a byte offset in the buffer. If it is an array, check the second item: if the second item is "chars", multiply the first number by the size of a character in bytes. If the second item is "bytes", treat it as a byte offset in the buffer.
    if isinstance(offset, int):
        ranges[i] = [start, end, offset]
    elif isinstance(offset, list):
        if offset[1] == 'chars':
            ranges[i] = [start, end, offset[0] * char_size]
        elif offset[1] == 'bytes':
            ranges[i] = [start, end, offset[0]]
        else:
            raise ValueError("Unknown offset type " + str(offset[1]))

out_bitmap_buffer = bytearray()
# Order all the bitmap data according to the ranges
for r in ranges:
    sidx = r[2]
    eidx = sidx + (r[1] - r[0] + 1) * char_size
    actual_idx = len(out_bitmap_buffer)
    out_bitmap_buffer.extend(buffer[sidx:eidx])
    r[2] = actual_idx

# Combine all the contiguous ranges
combined_ranges = []
if len(ranges) > 0:
    current_range = ranges[0]
    for r in ranges[1:]:
        if r[0] == current_range[1]:
            current_range = (current_range[0], r[1], current_range[2])
        else:
            combined_ranges.append(current_range)
            current_range = r
    combined_ranges.append(current_range)
ranges = combined_ranges

compressed_buffer = compress_bytes(out_bitmap_buffer)
compressed = False
rngs_buf = bytearray()
for r in ranges:
    rngs_buf.extend(r[0].to_bytes(2, byteorder='little'))
    rngs_buf.extend(r[1].to_bytes(2, byteorder='little'))
    rngs_buf.extend(r[2].to_bytes(4, byteorder='little'))
rngs_buf_comp = compress_bytes(rngs_buf)
rngs_is_comp = False

compr_all_blksz = (len(compressed_buffer) + len(rngs_buf_comp)) // FS_BLKSZ + 1
compr_bmp_only_blksz = (len(compressed_buffer) + len(rngs_buf)) // FS_BLKSZ + 1
compr_rngs_only_blksz = (len(out_bitmap_buffer) + len(rngs_buf_comp)) // FS_BLKSZ + 1
raw_blksz = (len(out_bitmap_buffer) + len(rngs_buf)) // FS_BLKSZ + 1

print("Raw size =", raw_blksz, "blocks")
print("Compress BMP only =", compr_bmp_only_blksz, "blocks")
print("Compress Rngs only =", compr_rngs_only_blksz, "blocks")
print("Compress both =", compr_all_blksz, "blocks")

# Pick which of the sizes is smallest and set rngs_is_comp and compressed flags accordingly. If two sizes are equal, prefer the one with less flags set, to balance between CPU time when loading and file size.
if compr_all_blksz < compr_bmp_only_blksz and compr_all_blksz < compr_rngs_only_blksz and compr_all_blksz < raw_blksz:
    print("Compressing both tables")
    compressed = True
    rngs_is_comp = True
elif compr_bmp_only_blksz < compr_rngs_only_blksz and compr_bmp_only_blksz < raw_blksz:
    print("Compressing only BMPs")
    compressed = True
elif compr_rngs_only_blksz < raw_blksz:
    print("Compressing only RNGs")
    rngs_is_comp = True
else:
    print("Not compressing")


outf = open(OUTNAME, 'wb')
# START OF HEADER SECTION (MoFo)
outf.write(b'MoFo') # magic
outf.write((1).to_bytes(2, byteorder = 'little')) # version
# encoding
if fdesc["encoding"] == "utf16":
    outf.write(b'\x01')
elif fdesc["encoding"] == "ascii":
    outf.write(b'\x00')
else:
    print("WARN: unknown encoding ID " + fdesc["encoding"])
    outf.write(b'\x00')
# glyph format
if fdesc['glyph_format'] == 'horizontal':
    outf.write(b'\x00')
else:
    outf.write(b'\x01')
outf.write(fdesc['cursor'].to_bytes(2, byteorder = 'little')) # cursor
outf.write(fdesc['invalid'].to_bytes(2, byteorder = 'little')) # invalid char
outf.write(fdesc['width'].to_bytes(1, byteorder = 'little'))
outf.write(fdesc['height'].to_bytes(1, byteorder = 'little'))
# END OF HEADER SECTION
# START OF RANGES SECTION (Rngs)
if not rngs_is_comp:
    outf.write(b'Rngs') # magic
    outf.write((len(rngs_buf)).to_bytes(4, byteorder = 'little'))
    outf.write(len(ranges).to_bytes(4, byteorder = 'little'))
    outf.write(rngs_buf)
else:
    outf.write(b'RngZ') # magic
    outf.write((len(rngs_buf_comp)).to_bytes(4, byteorder = 'little'))
    outf.write(len(rngs_buf).to_bytes(4, byteorder = 'little'))
    outf.write(rngs_buf_comp)
# END OF RANGES SECTION
# START OF BITMAP SECTION
outf.write(b'BMPZ' if compressed else b'BMPs')
outf.write(len(compressed_buffer if compressed else out_bitmap_buffer).to_bytes(4, byteorder = 'little'))
outf.write(len(out_bitmap_buffer).to_bytes(4, byteorder = 'little'))
outf.write(compressed_buffer if compressed else out_bitmap_buffer)
# END OF BITMAP SECTION
outf.close()