#pragma once
#include <views/framework.h>
#include <sound/beeper.h>
#include <service/alarm.h>
#include "proto/navmenu.h"

class AppShimAlarmEditor: public ProtoShimNavMenu {
public:
    AppShimAlarmEditor(Beeper*);
    void pop_renderable(transition_type_t = TRANSITION_SLIDE_HORIZONTAL_RIGHT);

private:
    Beeper * beeper;
};