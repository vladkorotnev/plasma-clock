#pragma once
#include <sensor/sensor.h>
#include <sound/beeper.h>
#include <graphics/screenshooter.h>

/// Set up and start the web admin service.
void admin_panel_prepare(SensorPool*, Beeper*);