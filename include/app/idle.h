#pragma once
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sensor/sensor.h>

void app_idle_prepare(SensorPool*, Beeper*, NewSequencer*);
void app_idle_draw(FantaManipulator*);
void app_idle_process();

class AppShimIdle: public Renderable {
public:
    AppShimIdle(SensorPool*sp, Beeper*b, NewSequencer*s) {
        app_idle_prepare(sp, b, s);
    }

    void render(FantaManipulator*fb) {
        app_idle_draw(fb);
    }

    void step() {
        app_idle_process();
    }
};