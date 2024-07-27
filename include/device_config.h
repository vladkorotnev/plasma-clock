#ifndef FEATUREFLAG_H_
#define FEATUREFLAG_H_

#define HAS(x) defined(HAS_##x)

// ---- SOFTWARE FEATURE FLAGS
#define HAS_WORDNIK_API
// #define HAS_OTAFVU
// #define HAS_SWITCHBOT_METER_INTEGRATION
#define HAS_BALANCE_BOARD_INTEGRATION

// ---- HARDWARE

#ifdef ESP32
    #define HAS_BLUETOOTH_LE
    #define HAS_BLUETOOTH_CLASSIC
#endif

#ifdef DEVICE_PLASMA_CLOCK
#include <devices/big_clock.h>
#endif

#ifdef DEVICE_MICROPISOS
#include <devices/smol_clock.h>
#endif

// ---- DEPENDENCY RULES
#if !HAS(BLUETOOTH_LE)
    #if HAS(SWITCHBOT_METER_INTEGRATION)
        #error Switchbot requires Bluetooth LE
    #endif
#endif

#if !HAS(BLUETOOTH_CLASSIC)
    #if HAS(BALANCE_BOARD_INTEGRATION)
        #error Balance Board requires Bluetooth Classic
    #endif
#endif

#endif