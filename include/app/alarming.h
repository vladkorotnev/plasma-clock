#pragma once
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <sensor/sensor.h>

void app_alarming_prepare(Beeper*);
void app_alarming_draw(FantaManipulator*);
void app_alarming_process();