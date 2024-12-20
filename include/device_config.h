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

// WIP: Pixel Cave the game
// #define HAS_PIXEL_CAVE

// Enable serial MIDI input
// #define HAS_SERIAL_MIDI

// Enable the yukkuri voice talking clock
#ifdef LIBAQUESTALK_FOUND
#define HAS_AQUESTALK
#else
#warning "AquesTalk not found, TTS disabled. See `./lib/nonfree-aquestalk/README.md` on how to add the library correctly."
#endif

// Enable the scroll string between cycles
// #define HAS_FREE_TEXT_SCROLL

// Disable the faux brightness reduction for some UI elements by drawing them only every other frame
// #define COMPOSABLE_NO_EVENODD

// Enable the split-screen app host, where idle screen is always visible on the right and the menu/apps are on the left
// Useful only on 192px+ wide devices, and automatically enabled on such (see the end of this file)
// #define HAS_SPLIT_SCREEN_APPHOST

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

// Enable the split-screen app host, only useful on long screens
#if HWCONF_DISPLAY_WIDTH_PX >= 192
    #define HAS_SPLIT_SCREEN_APPHOST
#endif

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