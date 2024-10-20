#pragma once
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sensor/sensor.h>
#include <sound/yukkuri.h>

void app_idle_init(SensorPool*, Beeper*, NewSequencer*, Yukkuri*);
void app_idle_prepare();
void app_idle_draw(FantaManipulator*);
void app_idle_process();

class AppShimIdle: public Renderable {
public:
    AppShimIdle(SensorPool*sp, Beeper*b, NewSequencer*s, Yukkuri*y) {
        app_idle_init(sp, b, s, y);
    }

    void prepare() {
        app_idle_prepare();
    }

    void render(FantaManipulator*fb) {
        app_idle_draw(fb);
    }

    void step() {
        app_idle_process();
    }
};