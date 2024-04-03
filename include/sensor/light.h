#include "sensor.h"
#include <hal/gpio_types.h>
#pragma once

class AmbientLightSensor: public PoolableSensor {
public:
    AmbientLightSensor(gpio_num_t pin);
    int read();

    bool initialize();
    bool teardown();
    bool poll(int *);
private:
    gpio_num_t active_pin;
};