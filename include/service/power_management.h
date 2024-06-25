#pragma once
#include <sensor/sensors.h>
#include <display/md_plasma.h>
#include <sound/beeper.h>

#define PM_INTERVAL 500
#define PM_LIGHTNESS_THRESH_UP 1000
#define PM_LIGHTNESS_THRESH_DOWN 700

#define PM_MOTIONLESS_TURN_OFF_DELAY 5 * 60 * 1000
#define PM_MOTIONLESS_HV_SHUTDOWN_DELAY 10 * 60 * 1000

/// @brief Start the power management service
void power_mgmt_start(SensorPool*, MorioDenkiPlasmaDriver*, Beeper*);
/// @brief Suspend the power management service and enter the full power mode
void power_mgmt_pause();
/// @brief Resume the power management service after it was paused
void power_mgmt_resume();