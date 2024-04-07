#pragma once
#include <sensor/sensors.h>
#include <plasma/iface.h>
#include <sound/beeper.h>

#define PM_INTERVAL 500
#define PM_LIGHTNESS_THRESH_UP 2000
#define PM_LIGHTNESS_THRESH_DOWN 1500

#define PM_MOTIONLESS_TURN_OFF_DELAY 5 * 60 * 1000
#define PM_MOTIONLESS_HV_SHUTDOWN_DELAY 10 * 60 * 1000

void power_mgmt_start(SensorPool*, PlasmaDisplayIface*, Beeper*);
void power_mgmt_pause();
void power_mgmt_resume();