#ifndef FEATUREFLAG_H_
#define FEATUREFLAG_H_

#define HAS(x) defined(HAS_##x)

// ---- SOFTWARE FEATURE FLAGS

// Enable the playground app for shits and giggles
// #define HAS_PLAYGROUND

// Enable the Wordnik API and Word Of The Day screen. UNSTABLE: Uses a lot of RAM for HTTPS.
#define HAS_WORDNIK_API

// Enable over-the-air firmware version upgrade
// #define HAS_OTAFVU

// Enable HTTP firmware upgrade
#define HAS_HTTPFVU
// Set the HTTP firmware upgrade default server, if not present in .env
#ifndef FVU_SERVER_URL
#define FVU_SERVER_URL "http://pis-os.genjit.su/fvudata"
#endif

// DEPRECATED: Enable Switchbot Meter temperature probing. UNSTABLE: Uses NimBLE so a lot of RAM, disconnects over time.
// #define HAS_SWITCHBOT_METER_INTEGRATION

// DEPRECATED: Enable Wii Balance Board measuring. UNSTABLE: Uses Bluedroid (a FUCKTON of RAM), periodic disconnects or reboots without leaving a stack trace.
// #define HAS_BALANCE_BOARD_INTEGRATION

// Enable the yukkuri voice talking clock
#define HAS_AQUESTALK

// Disable the faux brightness reduction for some UI elements by drawing them only every other frame
// #define COMPOSABLE_NO_EVENODD

// ---- HARDWARE

#ifdef ESP32
    #define HAS_BLUETOOTH_LE
    #define HAS_BLUETOOTH_CLASSIC
#endif

#ifdef DEVICE_PLASMA_CLOCK
#include <devices/big_clock.h>
#endif

#ifdef DEVICE_MINIPISOS
#include <devices/mid_clock.h>
#endif

#ifdef DEVICE_MINIPISOS_VFD
#include <devices/mid_clock_noritake.h>
#endif

#ifdef DEVICE_MINIPISOS_VFD_WIDE
#include <devices/mid_clock_noritake_wide.h>
#endif

#ifdef DEVICE_LEPISOS
#include <devices/led_clock.h>
#endif

#ifdef DEVICE_LONGPISOS
#include <devices/big_noritake.h>
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

#if HAS(AQUESTALK)
    #if !defined(LIBAQUESTALK_FOUND)
        #error libaquestalk was not found. See `./lib/nonfree-aquestalk/README.md` on how to add it correctly, or disable `HAS_AQUESTALK` feature flag.
    #endif
#endif

#if HWCONF_DISPLAY_WIDTH_PX >= 192
    #define HAS_SPLIT_SCREEN_APPHOST
#endif

#endif