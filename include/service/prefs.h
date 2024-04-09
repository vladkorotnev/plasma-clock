#pragma once
#include <Arduino.h>

typedef const char * prefs_key_t;

static constexpr prefs_key_t PREFS_KEY_WIFI_SSID = "ssid";
static constexpr prefs_key_t PREFS_KEY_WIFI_PASS = "pass";

static constexpr prefs_key_t PREFS_KEY_FPS_COUNTER = "fps_c";

static constexpr prefs_key_t PREFS_KEY_TICKING_SOUND = "ticking_sound";
static constexpr prefs_key_t PREFS_KEY_NO_SOUND_WHEN_OFF = "no_tick_off";

static constexpr prefs_key_t PREFS_KEY_LIGHTNESS_THRESH_UP = "bri_thr_up";
static constexpr prefs_key_t PREFS_KEY_LIGHTNESS_THRESH_DOWN = "bri_thr_dn";

static constexpr prefs_key_t PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS = "mot_off_s";
static constexpr prefs_key_t PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS = "mot_hv_off_s";

static constexpr prefs_key_t PREFS_KEY_WEATHER_LAT = "w_lat";
static constexpr prefs_key_t PREFS_KEY_WEATHER_LON = "w_lon";
static constexpr prefs_key_t PREFS_KEY_WEATHER_APIKEY = "w_apikey";
static constexpr prefs_key_t PREFS_KEY_WEATHER_INTERVAL_MINUTES = "w_interval_m";

void prefs_force_save();

String prefs_get_string(prefs_key_t);
void prefs_set_string(prefs_key_t, String);

int prefs_get_int(prefs_key_t);
void prefs_set_int(prefs_key_t, int);

bool prefs_get_bool(prefs_key_t);
void prefs_set_bool(prefs_key_t, bool);