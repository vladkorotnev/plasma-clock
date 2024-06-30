#pragma once
#include "view.h"
#include <graphics/fanta_manipulator.h>
#include <sensor/sensor.h>

class IndoorView: public Screen {
public:
    IndoorView(SensorPool*);
    void render(FantaManipulator*);
    int desired_display_time();

private:
    SensorPool * sensors;
    const font_definition_t * value_font;
};