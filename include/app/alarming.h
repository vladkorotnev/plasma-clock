#pragma once
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

void app_alarming_prepare(Beeper*);
void app_alarming_draw(FantaManipulator*);
void app_alarming_process();

class AppShimAlarming: public Renderable {
public:
    AppShimAlarming(Beeper*b) {
        beeper = b;
    }

    void prepare() {
        app_alarming_prepare(beeper);
    }

    void render(FantaManipulator*fb) {
        app_alarming_draw(fb);
    }

    void step() {
        app_alarming_process();
    }
private:
    Beeper *beeper;
};