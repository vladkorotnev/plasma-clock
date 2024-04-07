#pragma once
#include "view.h"
#include <plasma/fanta_manipulator.h>
#include <sensor/sensor.h>

class IndoorView: public Renderable {
public:
    IndoorView(SensorPool*, FantaManipulator*);
    void render();

private:
    SensorPool * sensors;
    FantaManipulator * fb;
    const font_definition_t * value_font;
};