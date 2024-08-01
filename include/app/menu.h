#pragma once
#include "proto/navmenu.h"
#include <views/framework.h>
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sound/sequencer.h>
#include <sensor/sensor.h>

class AppShimMenu: public ProtoShimNavMenu {
public:
    AppShimMenu(Beeper*, NewSequencer*);

    void prepare();
    void step();

    void pop_renderable(transition_type_t = TRANSITION_SLIDE_HORIZONTAL_RIGHT);

private:
    Beeper * beeper;
    TickType_t last_touch_time;
};