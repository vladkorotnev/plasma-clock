#pragma once
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

void app_idle_prepare(SensorPool*, Beeper*);
void app_idle_draw(FantaManipulator*);
void app_idle_process();