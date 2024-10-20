#-*- coding: utf-8 -*-

# Super jank converter from 8bit WAV into RLE PWM + preview
# Not reading the header or anything hence super jank

import sys

MARGIN = 4

fname = sys.argv[1]
sname = sys.argv[2]
oname = sys.argv[3] if len(sys.argv) >= 4 else None
sdata = open(fname, 'rb').read()
outf = None
if oname is not None:
    outf = open(oname, 'wb')
    outf.write(sdata[:0x28])

sdata = sdata[0x28::]
i = 0
min = 999
max = 0
sts = 0xFF
last_sts = 0xFF
rle_buf = [0]

def median(data):
    x = list(data)
    x.sort()
    mid = len(x) // 2
    return (x[mid] + x[~mid]) / 2.0

med = median(sdata)
print("Median", med)
HIGH = med + MARGIN
LO = med - MARGIN

while i < len(sdata):
    curSample = sdata[i]
    if curSample >= HIGH:
        sts = 255
    elif curSample <= LO:
        sts = 1
    if curSample < min and curSample > 0:
        min = curSample
    if curSample > max and curSample > 0:
        max = curSample
    if outf is not None:
        outf.write(bytes([sts]))
    if sts != last_sts:
        rle_buf.append(0)
        last_sts = sts
    if rle_buf[-1] == 255:
        rle_buf.append(0)
        rle_buf.append(0)
    rle_buf[-1] += 1
    i += 1

print(f"static const uint8_t {sname}_rle_data[] = {{" + str(rle_buf)[1::][:-1:] + "};")
print(f"static const rle_sample_t {sname} = {{ .sample_rate = 8000, .root_frequency = 524 /* C5 */, .length = {len(rle_buf)}, .mode = MIX_MODE_ADD, .rle_data = {sname}_rle_data }};")