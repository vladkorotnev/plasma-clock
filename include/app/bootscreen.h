#pragma once
#include "fonts.h"
#include <service/localize.h>
#include <service/prefs.h>
#include <views/framework.h>
#include <views/common/swoopie.h>
#include <algorithm>

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
