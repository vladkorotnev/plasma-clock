#include "sensor.h"
#include <AM232X.h>
#pragma once

class Am2322HumiditySensor: public PoolableSensor {
public:
    Am2322HumiditySensor(AM232X *);
    bool initialize();
    bool teardown();
    bool poll(int *);
private:
    AM232X* hw;
};

class Am2322TemperatureSensor: public PoolableSensor {
public:
    Am2322TemperatureSensor(AM232X *);
    bool initialize();
    bool teardown();
    bool poll(int *);
private:
    AM232X* hw;
};