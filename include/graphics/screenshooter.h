#pragma once
#include "fanta_manipulator.h"
#include <device_config.h>

class Screenshooter {
public:
    Screenshooter();
    bool capture(FantaManipulator*, const uint8_t ** pngDataBufOut, size_t * pngDataLenOut);
};