#pragma once
#include <Arduino.h>

typedef const char * prefs_key_t;

static constexpr prefs_key_t PREFS_KEY_WIFI_SSID = "ssid";
static constexpr prefs_key_t PREFS_KEY_WIFI_PASS = "pass";

static constexpr prefs_key_t PREFS_KEY_FPS_COUNTER = "fps_c";

static constexpr prefs_key_t PREFS_KEY_BLINK_SEPARATORS = "blink_separator";
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

static constexpr prefs_key_t PREFS_KEY_WORDNIK_APIKEY = "wd_apikey";
static constexpr prefs_key_t PREFS_KEY_WORDNIK_INTERVAL_MINUTES = "wd_interval_m";

static constexpr prefs_key_t PREFS_KEY_TRANSITION_TYPE = "s_transition";
static constexpr prefs_key_t PREFS_KEY_DISP_SCROLL_SPEED = "s_scrl_spd";

static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_CLOCK_SECONDS = "s_clock_s";
static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_INDOOR_SECONDS = "s_inside_s";
static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS = "s_remote_s";
static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS = "s_outside_s";
static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS = "s_wotd_s";
static constexpr prefs_key_t PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS = "s_foo_s";

static constexpr prefs_key_t PREFS_KEY_HOURLY_CHIME_ON = "h_chime_on";
static constexpr prefs_key_t PREFS_KEY_HOURLY_CHIME_START_HOUR = "h_chime_start";
static constexpr prefs_key_t PREFS_KEY_HOURLY_CHIME_STOP_HOUR = "h_chime_stop";
static constexpr prefs_key_t PREFS_KEY_HOURLY_CHIME_MELODY = "h_chime";
static constexpr prefs_key_t PREFS_KEY_FIRST_CHIME_MELODY = "h_1stchime";

static constexpr prefs_key_t PREFS_KEY_TIMEZONE = "tk_tz";
static constexpr prefs_key_t PREFS_KEY_TIMESERVER = "tk_ntp_serv";
static constexpr prefs_key_t PREFS_KEY_TIME_SYNC_INTERVAL_SEC = "tk_intv_s";

static constexpr prefs_key_t PREFS_KEY_FOOBAR_SERVER = "foo_svr";
static constexpr prefs_key_t PREFS_KEY_FOOBAR_PORT = "foo_prt";

static constexpr prefs_key_t PREFS_KEY_SWITCHBOT_METER_ENABLE = "wos_ena";
static constexpr prefs_key_t PREFS_KEY_SWITCHBOT_METER_MAC = "wos_mac";
static constexpr prefs_key_t PREFS_KEY_SWITCHBOT_EMULATES_LOCAL = "wos_emus";

void prefs_force_save();

String prefs_get_string(prefs_key_t, String def = String());
void prefs_set_string(prefs_key_t, String);

int prefs_get_int(prefs_key_t);
void prefs_set_int(prefs_key_t, int);

bool prefs_get_bool(prefs_key_t);
void prefs_set_bool(prefs_key_t, bool);