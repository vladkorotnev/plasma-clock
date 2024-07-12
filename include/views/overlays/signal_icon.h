#pragma once
#include <views/common/view.h>
#include <sensor/sensor.h>

class SignalStrengthIcon: public Renderable {
public:
    SignalStrengthIcon(SensorPool*);

    void step();
    void render(FantaManipulator*);

private:
    SensorPool* sensors;
    int currentRssi;
    bool isShowing;
    sprite_t currentIcon;
};