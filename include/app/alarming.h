#pragma once
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sensor/sensor.h>

void app_alarming_prepare(NewSequencer*);
void app_alarming_draw(FantaManipulator*);
void app_alarming_process();

class AppShimAlarming: public Renderable {
public:
    AppShimAlarming(NewSequencer*b) {
        sequencer = b;
    }

    void prepare() {
        app_alarming_prepare(sequencer);
    }

    void render(FantaManipulator*fb) {
        app_alarming_draw(fb);
    }

    void step() {
        app_alarming_process();
    }
private:
    NewSequencer *sequencer;
};