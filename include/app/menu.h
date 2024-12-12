#pragma once
#include "proto/navmenu.h"
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sound/yukkuri.h>
#include <sensor/sensor.h>
#include <sensor/light.h>

class AppShimMenu: public ProtoShimNavMenu {
public:
    AppShimMenu(Beeper*, NewSequencer*, Yukkuri*, AmbientLightSensor*);

    void prepare() override;
    void render(FantaManipulator*) override;
    void step() override;

    void pop_renderable(transition_type_t = TRANSITION_SLIDE_HORIZONTAL_RIGHT);

private:
    Beeper * beeper;
    Yukkuri * yukkuri;
    TickType_t last_touch_time;
    int last_width;
};