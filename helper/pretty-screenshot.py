#!/bin/env python
# -*- coding: utf-8 -*-

import sys
from pathlib import Path

IMG_IN=sys.argv[1]
IMG_OUT=sys.argv[2] if len(sys.argv) > 2 else str(Path(IMG_IN).with_suffix(''))+"_pretty.png"

PIX_COLOR=(0, 210, 242)
PIX_SIZE=4
PIX_PITCH_H=1
PIX_PITCH_W=1

from PIL import Image, ImageFilter
im = Image.open(IMG_IN, 'r')
src = im.load()

width, height = im.size

out_img = Image.new(mode="RGB", size=(width * (PIX_SIZE + PIX_PITCH_H), height * (PIX_SIZE + PIX_PITCH_H)))

for y in range(height):
    for x in range(width):
        v = src[x, y]
        for dy in range(PIX_SIZE):
            for dx in range(PIX_SIZE):
                out_x = x * (PIX_SIZE + PIX_PITCH_W) + dx
                out_y = y * (PIX_SIZE + PIX_PITCH_H) + dy
                if out_x < out_img.width and out_y < out_img.height:
                    out_img.putpixel((out_x, out_y), PIX_COLOR if v > 0 else (0, 0, 0))

out_img.filter(ImageFilter.GaussianBlur(radius=PIX_SIZE/10)).save(IMG_OUT)