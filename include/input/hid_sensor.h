#pragma once
#include <sensor/sensor.h>
#include "keys.h"

class HidActivitySensor: public TimerSensor {
public:
    HidActivitySensor(): TimerSensor(pdMS_TO_TICKS(3000)) {}

    bool poll(int * result) {
        if(hid_test_key_any() != KEYSTATE_RELEASED) trigger();
        return TimerSensor::poll(result);
    }
};