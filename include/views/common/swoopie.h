#pragma once
#include <views/framework.h>

class Swoopie: public Renderable {
public:
    Swoopie() {}
    int value = -1;
    int minimum = 0;
    int maximum = 100;
    int y_offset = 12;

    void prepare() override {
        headPos = -5;
        tailPos = headPos - minLength;
        framecounter = 0;
    }

    void render(FantaManipulator* fb) override {
        if(value == -1) {
            render_indeterminate(fb);
        } else {
            render_determinate(fb);
        }
    }

private:
    int headPos = -1;
    int tailPos = -5;
    int maxLength = 25;
    int minLength = 2;
    int speed = 1;
    int framecounter = 0;

    void render_determinate(FantaManipulator *fb) {
        int pixels = ((fb->get_width() * (value - minimum)) / (maximum - minimum));
        fb->line(0, y_offset, pixels, y_offset, true);
        fb->line(pixels, y_offset, fb->get_width(), y_offset, false);
        if(pixels < fb->get_width()) {
            int x, y;
            if(framecounter <= 30) {
                x = 1;
                y = 0;
            } 
            else if(framecounter <= 60) {
                x = 1;
                y = -1;
            }
            else if(framecounter <= 100) {
                x = 2;
                y = -1;
            }
            else if(framecounter <= 150) {
                x = 2;
                y = 0;
            }
            else if(framecounter <= 170) {
                x = 2;
                y = 1;
            }
            else if(framecounter <= 190) {
                x = 1;
                y = 1;
            }
            else {
                x = 1;
                y = 0;
            }

            fb->plot_pixel(pixels + x, y_offset + y, true);
        }
        framecounter++;
        if(framecounter == 210) {
            framecounter = 0;
        }
    }

    void render_indeterminate(FantaManipulator *fb) {
        fb->line(0, y_offset, fb->get_width(), y_offset, false);
        fb->line(headPos - 2, y_offset, tailPos + 2, y_offset);
        fb->plot_pixel(headPos, y_offset, true);
        fb->plot_pixel(tailPos, y_offset, true);

        if(tailPos >= fb->get_width()) {
            headPos = -5;
            tailPos = headPos - minLength;
            speed = 1;
        } else {
            if(headPos - tailPos < maxLength) {
                headPos += speed;
                if(framecounter % 3 == 0) tailPos += 1;
            }
            else if(headPos - tailPos == maxLength && headPos <= fb->get_width()) {
                headPos += speed;
                tailPos += speed;
            }
            else if(tailPos <= fb->get_width()) {
                tailPos += speed;
            }
        }

        framecounter++;
        if(framecounter % 20 == 0) {
            if(speed < 10 && headPos <= fb->get_width()) {
                speed++;
            }
            else if(speed > 5 && tailPos <= fb->get_width()) {
                speed--;
            }
        }
    }
};