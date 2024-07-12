#pragma once
#include <views/framework.h>
#include <service/prefs.h>
#include <fonts.h>

class FpsCounter: public Renderable {
public:
    FpsCounter(DisplayFramebuffer *disp) {
        active = false;
        display = disp;
    }

    void render(FantaManipulator*fb) {
        if(!active) return;

        fb->put_string(&fps_counter_font, buf, 0, 0);
    }

    void step() {
        active = prefs_get_bool(PREFS_KEY_FPS_COUNTER);
        if(active) itoa(display->get_fps(), buf, 10);
    }
private:
    char buf[4] = { 0 };
    bool active;
    DisplayFramebuffer * display;
};