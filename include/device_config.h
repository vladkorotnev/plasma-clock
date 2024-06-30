#ifndef FEATUREFLAG_H_
#define FEATUREFLAG_H_

#define HAS(x) defined(HAS_##x)

// ---- SOFTWARE FEATURE FLAGS
#define HAS_WORDNIK_API
// #define HAS_OTAFVU // <- disabled to expand partition (see partitions.csv), not critical for me for now
#define HAS_SWITCHBOT_METER_INTEGRATION

// ---- HARDWARE

#ifdef ESP32
    #define HAS_BLUETOOTH_LE
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
        #undef HAS_SWITCHBOT_METER_INTEGRATION
    #endif
#endif

#endif