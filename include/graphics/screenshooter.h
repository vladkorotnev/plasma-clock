#pragma once
#include "fanta_manipulator.h"
#include <device_config.h>

class Screenshooter {
public:
    Screenshooter(FantaManipulator *);
    void start_server(uint16_t port);
    void stop_server();

private:
    TaskHandle_t hServerTask = NULL;
    FantaManipulator * framebuffer = nullptr;
};