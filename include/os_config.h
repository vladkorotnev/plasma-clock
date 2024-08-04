#pragma once
#include <FreeRTOSConfig.h>

// ----- Task Priorities -----

#define pisosTASK_PRIORITY_CONSOLE 2
#define pisosTASK_PRIORITY_REMOTE_DESKTOP 3
#define pisosTASK_PRIORITY_FRAMEBUFFER (configMAX_PRIORITIES - 1)
#define pisosTASK_PRIORITY_WAVEOUT (configMAX_PRIORITIES - 1)
#define pisosTASK_PRIORITY_KEYPAD (configMAX_PRIORITIES - 3)
#define pisosTASK_PRIORITY_WEBADMIN 4
#define pisosTASK_PRIORITY_OTAFVU 10
#define pisosTASK_PRIORITY_SENSOR 3
#define pisosTASK_PRIORITY_ALARM 4
#define pisosTASK_PRIORITY_BALANCEBOARD 2
#define pisosTASK_PRIORITY_FOOBAR_CLIENT 2
#define pisosTASK_PRIORITY_POWER_MANAGEMENT 4
#define pisosTASK_PRIORITY_WORDNIK 1
#define pisosTASK_PRIORITY_WEATHER 1

// ----- WaveOut channels -----

#define pisosWAVE_CHANNEL_BEEPER 0
#define pisosWAVE_CHANNEL_SEQUENCER 1