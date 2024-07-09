#pragma once
#include "sensor.h"

class RssiSensor: public PoolableSensor {
public:
    RssiSensor();
    bool poll(int*);
};