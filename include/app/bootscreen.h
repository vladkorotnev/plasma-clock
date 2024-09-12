#pragma once
#include "fonts.h"
#include <service/localize.h>
#include <service/prefs.h>
#include <views/framework.h>
#include <algorithm>

class Swoopie: public Renderable {
public:
    Swoopie() {}

    void prepare() override {
        headPos = -5;
        tailPos = headPos - minLength;
        framecounter = 0;
    }

    void render(FantaManipulator* fb) override {
        fb->line(0, y_offs, fb->get_width(), y_offs, false);
        fb->line(headPos - 2, y_offs, tailPos + 2, y_offs);
        fb->plot_pixel(headPos, y_offs, true);
        fb->plot_pixel(tailPos, y_offs, true);

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

private:
    int y_offs = 12;

    int headPos = -1;
    int tailPos = -5;
    int maxLength = 25;
    int minLength = 2;
    int speed = 1;
    int framecounter = 0;
};

class ProtoBootscreen: public Composite {
protected:
    ProtoBootscreen(const char * title) {
        wants_clear_surface = true;
        
        progBar = new Swoopie();
        label = new StringScroll(&keyrus0808_font, title);

        label->stopped = true;
        label->x_offset = (DisplayFramebuffer::width/2 - label->string_width / 2);

        add_composable(label);
        add_subrenderable(progBar);
    }

private:
    Swoopie * progBar;
    StringScroll * label;
};

class Bootscreen: public ProtoBootscreen {
public:
    Bootscreen(): ProtoBootscreen(PRODUCT_NAME " " PRODUCT_VERSION) {}
};

class RebootScreen: public ProtoBootscreen {
public:
    RebootScreen(): ProtoBootscreen(localized_string("Restarting...")) {}
    void step() override {
        if(!did_the_thing && delay == 0) {
            did_the_thing = true;
            prefs_force_save();
            ESP.restart();
        } else if(delay > 0) {
            delay--;
        }
    }
private:
    bool did_the_thing = false;
    int delay = 100;
};