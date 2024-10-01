#pragma once
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sensor/sensor.h>
#include <sound/yukkuri.h>

void app_idle_prepare(SensorPool*, Beeper*, NewSequencer*, Yukkuri*);
void app_idle_draw(FantaManipulator*, RenderPlane rp);
void app_idle_process();

class AppShimIdle: public Renderable {
public:
    AppShimIdle(SensorPool*sp, Beeper*b, NewSequencer*s, Yukkuri*y) {
        app_idle_prepare(sp, b, s, y);
    }

    void render_plane(FantaManipulator*fb, RenderPlane rp) override {
        app_idle_draw(fb, rp);
    }

    void step() {
        app_idle_process();
    }
};