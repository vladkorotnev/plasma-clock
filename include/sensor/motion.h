#include "sensor.h"
#include <hal/gpio_types.h>
#pragma once

class MotionSensor: public PoolableSensor {
public:
    MotionSensor(gpio_num_t pin);
    bool read();

    bool initialize();
    bool teardown();
    bool poll(int *);
    
private:
    gpio_num_t active_pin;
};