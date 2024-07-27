#!/bin/env python
# -*- coding: utf-8 -*-
import tkinter as tk
from tkinter.filedialog import asksaveasfilename
import sys, socket, struct
from PIL import Image, ImageFilter
import pdb
import threading
import time

IP=sys.argv[1]
PORT=3939
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.1)

PIX_COLOR=(0, 210, 242)
PIX_SIZE=4
PIX_PITCH_H=1
PIX_PITCH_W=1

PRESS_STS=0
RELEASE_STS=0
DISP_STS=[]
DISP_RES=(0,0)

REC_IMGS=[]
REC_STS=False

def _buf_to_pillow(buf, width, height):
    global PIX_COLOR, PIX_PITCH_H, PIX_PITCH_W, PIX_SIZE
    out_img = Image.new(mode="RGB", size=(width * (PIX_SIZE + PIX_PITCH_H), height * (PIX_SIZE + PIX_PITCH_H)))
    for x in range(width):
        for y in range(height):
            tidx = (x * (height // 8)) + (y // 8)
            col = buf[tidx]
            v = col & (1 << (y % 8))
            for dx in range(PIX_SIZE):
                for dy in range(PIX_SIZE):
                    out_x = x * (PIX_SIZE + PIX_PITCH_W) + dx
                    out_y = y * (PIX_SIZE + PIX_PITCH_H) + dy
                    if out_x < out_img.width and out_y < out_img.height:
                        out_img.putpixel((out_x, out_y), PIX_COLOR if v > 0 else (0, 0, 0))
    return out_img.filter(ImageFilter.GaussianBlur(radius=PIX_SIZE/10))

def _buf_to_screen(buf, width, height):
    global PIX_COLOR, PIX_PITCH_H, PIX_PITCH_W, PIX_SIZE, canvas
    canvas.delete('all')
    for x in range(width):
        for y in range(height):
            tidx = (x * (height // 8)) + (y // 8)
            col = buf[tidx]
            v = col & (1 << (y % 8))
            canvas.create_rectangle(x * (PIX_SIZE+PIX_PITCH_W), y * (PIX_SIZE+PIX_PITCH_H), x * (PIX_SIZE+PIX_PITCH_W)+PIX_SIZE, y * (PIX_SIZE+PIX_PITCH_H)+PIX_SIZE,fill=("white" if v>0 else "black"))

def render_screenshot():
    global DISP_STS, DISP_RES
    temp = DISP_STS[:]
    width, height = DISP_RES
    f = asksaveasfilename(initialfile = 'screen.png', defaultextension=".png",filetypes=[("PNG Image","*.png")])
    if f == "":
        return
    out_img = _buf_to_pillow(temp, width, height)
    out_img.save(f)

def rec_toggle():
    global rec_button, REC_STS, REC_IMGS, DISP_RES
    if not REC_STS:
        REC_STS = True
        rec_button.configure(bg = "red", text="STOP")
    else:
        REC_STS = False
        rec_button.configure(bg = "SystemButtonFace", text="Record")
        width, height = DISP_RES
        f = asksaveasfilename(initialfile = 'screen.gif', defaultextension=".gif",filetypes=[("Animated GIF","*.gif")])
        if f == "":
            REC_IMGS = []
            return
        REC_IMGS = list(map(lambda buf: _buf_to_pillow(buf, width, height), REC_IMGS))
        REC_IMGS[0].save(f, save_all=True, append_images=REC_IMGS[1:], optimize=False, duration=34, loop=0)
        REC_IMGS = []


def ctl_btn_pressed(event):
    global PRESS_STS
    PRESS_STS |=  event

def ctl_btn_released(event):
    global RELEASE_STS
    RELEASE_STS |= event

def tx_pkt():
    global PRESS_STS, RELEASE_STS, sock, IP, PORT
    pkt = struct.pack("HII", 0x3939, PRESS_STS, RELEASE_STS)
    PRESS_STS = 0
    RELEASE_STS = 0
    sock.sendto(pkt, (IP, PORT))
    rx_pkt()

def rx_pkt():
    global sock, IP, PORT, DISP_STS, DISP_RES, REC_STS, REC_IMGS
    try:
        data, addrport = sock.recvfrom(255)
    except TimeoutError:
        return
    if addrport[0] != IP or addrport[1] != PORT:
        return
    if data[0] != 0x88 or data[1] != 0x88:
        print("Bad magic")
        return
    size = data[2:6:]
    res = struct.unpack('HH', size)
    if res != DISP_RES:
        print("New resolution", res)
        DISP_RES = res
        if REC_STS:
            REC_IMGS = []
    bitmap = data[6::]
    DISP_STS = bitmap
    if REC_STS:
        REC_IMGS.append(bitmap)

def send_packet_loop():
    while True:
        tx_pkt()
        time.sleep(0.033)

def drawing():
    global DISP_RES, DISP_STS, window
    _buf_to_screen(DISP_STS, DISP_RES[0], DISP_RES[1])
    window.after(33, drawing)


# Create the main window
window = tk.Tk()
window.title("PIS-OS Remote")
window.resizable(False, False)

# Set window size
window.geometry("520x150")
window.grid_columnconfigure(0, weight=1, uniform="foo")
window.grid_columnconfigure(1, weight=1, uniform="foo")
window.grid_columnconfigure(2, weight=1, uniform="foo")
window.grid_columnconfigure(3, weight=1, uniform="foo")

# Create the canvas
canvas = tk.Canvas(window, width=510, height=82, bg="black")
canvas.grid(row = 1, column = 0, columnspan=4)

left_button = tk.Button(window, text="←")
left_button.grid(row = 2, column = 0)
left_button.bind("<ButtonPress-1>", lambda _: ctl_btn_pressed(4))
left_button.bind("<ButtonRelease-1>", lambda _: ctl_btn_released(4))

top_button = tk.Button(window, text="↑")
top_button.grid(row = 2, column = 1)
top_button.bind("<ButtonPress-1>", lambda _: ctl_btn_pressed(1))
top_button.bind("<ButtonRelease-1>", lambda _: ctl_btn_released(1))

bottom_button = tk.Button(window, text="↓")
bottom_button.grid(row = 2, column = 2)
bottom_button.bind("<ButtonPress-1>", lambda _: ctl_btn_pressed(2))
bottom_button.bind("<ButtonRelease-1>", lambda _: ctl_btn_released(2))

right_button = tk.Button(window, text="→")
right_button.grid(row = 2, column = 3)
right_button.bind("<ButtonPress-1>", lambda _: ctl_btn_pressed(8))
right_button.bind("<ButtonRelease-1>", lambda _: ctl_btn_released(8))

ss_button = tk.Button(window, text="Screenshot", command=render_screenshot)
ss_button.grid(row = 0, column = 0, columnspan = 2)

rec_button = tk.Button(window, text="Record", command=rec_toggle)
rec_button.grid(row = 0, column = 2, columnspan = 2)

# Start the background thread for sending packets
packet_thread = threading.Thread(target=send_packet_loop)
packet_thread.daemon = True
packet_thread.start()

# Start the main event loop
window.after(33, drawing)
window.mainloop()
