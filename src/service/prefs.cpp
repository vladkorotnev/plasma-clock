#include "service/prefs.h"
#include <Preferences.h>
#include "nvs_flash.h"
#include <device_config.h>

static char LOG_TAG[] = "PREF";
static Preferences * store = nullptr;
static constexpr const char * STORE_DOMAIN = "pisos_prefs";

static constexpr prefs_key_t PREFS_KEY_INITIAL_SETTING_DONE = "init";

inline void init_store_if_needed() {
    if(store == nullptr) {
        ESP_LOGI(LOG_TAG, "Initialize");
        store = new Preferences();
        nvs_flash_init(); // <- sometimes language settings are being checked before Arduino finished initializing, so we need to bring NVS up on our own
        if(!store->begin(STORE_DOMAIN)) {
            delete store;
            store = nullptr;
        } else {
            if(!prefs_get_bool(PREFS_KEY_INITIAL_SETTING_DONE)) {
                // Write some sane settings for some of the keys
                prefs_set_int(PREFS_KEY_ALARM_SNOOZE_MINUTES, 10);
                #if HAS(LIGHT_SENSOR)
                    prefs_set_int(PREFS_KEY_BRIGHTNESS_MODE, BRIGHTNESS_AUTOMATIC);
                #endif
                prefs_set_int(PREFS_KEY_VOICE_SPEED, 100);
                prefs_set_bool(PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL, true);
                prefs_set_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR, true);
                prefs_set_bool(PREFS_KEY_HOURLY_CHIME_ON, true);
                prefs_set_bool(PREFS_KEY_TICKING_SOUND, true);
                prefs_set_bool(PREFS_KEY_HOURLY_CHIME_ON, true);
                prefs_set_int(PREFS_KEY_HOURLY_CHIME_START_HOUR, 9);
                prefs_set_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 21);
                prefs_set_int(PREFS_KEY_ALARM_MAX_DURATION_MINUTES, 60);
                prefs_set_int(PREFS_KEY_HOURLY_CHIME_MELODY, 3);
                prefs_set_bool(PREFS_KEY_VOICE_SPEAK_ON_HEADPAT, true);
                prefs_set_bool(PREFS_KEY_WEATHER_OVERLAY, true);
                prefs_set_bool(PREFS_KEY_TIMESERVER_ENABLE, true);
                #ifdef WORDNIK_API_KEY
                    prefs_set_string(PREFS_KEY_WORDNIK_APIKEY, String(WORDNIK_API_KEY));
                #endif
                #ifdef WEATHER_API_KEY
                    prefs_set_string(PREFS_KEY_WEATHER_APIKEY, String(WEATHER_API_KEY));
                #endif
                #ifdef WEATHER_LAT
                    prefs_set_string(PREFS_KEY_WEATHER_LAT, String(WEATHER_LAT));
                #endif
                #ifdef WEATHER_LON
                    prefs_set_string(PREFS_KEY_WEATHER_LON, String(WEATHER_LON));
                #endif
                #ifdef FVU_SERVER_URL
                    prefs_set_string(PREFS_KEY_FVU_SERVER, String(FVU_SERVER_URL));
                    prefs_set_bool(PREFS_KEY_FVU_AUTO_CHECK, true);
                    prefs_set_bool(PREFS_KEY_FVU_AUTO_INSTALL, true);
                    prefs_set_int(PREFS_KEY_FVU_AUTO_CHECK_INTERVAL_MINUTES, 240);
                #endif
                #if defined(ADMIN_LOGIN) && defined(ADMIN_PASS)
                    prefs_set_string(PREFS_KEY_ADMIN_ID, String(ADMIN_LOGIN));
                    prefs_set_string(PREFS_KEY_ADMIN_PASS, String(ADMIN_PASS));
                #endif
                #if defined(DEFAULT_WIFI_SSID) && defined(DEFAULT_WIFI_PASS)
                    prefs_set_string(PREFS_KEY_WIFI_SSID, DEFAULT_WIFI_SSID);
                    prefs_set_string(PREFS_KEY_WIFI_PASS, DEFAULT_WIFI_PASS);
                #endif
                prefs_set_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS, 15);
                prefs_set_int(PREFS_KEY_SCRN_TIME_NEXT_ALARM_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_FORECAST_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_PRECIPITATION_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_FORECAST_SECONDS, 5);
                prefs_set_int(PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS, 5);

                prefs_set_bool(PREFS_KEY_INITIAL_SETTING_DONE, true);
            }
        }
    }
}

void prefs_force_save() {
    if(store != nullptr) {
        store->end();
        store = nullptr;
    }
}

String prefs_get_string(prefs_key_t key, String def) {
    init_store_if_needed();
    String val = store->getString(key);
    if(val.length() > 0) {
        return val;
    } else {
        return def;
    }
}
void prefs_set_string(prefs_key_t key, String val) {
    init_store_if_needed();
    ESP_LOGV(LOG_TAG, "Set %s = %s", key, val.c_str());
    store->putString(key, val);
}

int prefs_get_int(prefs_key_t key) {
    init_store_if_needed();
    int val = store->getInt(key);
    return val;
}
void prefs_set_int(prefs_key_t key, int val) {
    init_store_if_needed();
    ESP_LOGV(LOG_TAG, "Set %s = %i", key, val);
    store->putInt(key, val);
}

bool prefs_get_bool(prefs_key_t key) {
    init_store_if_needed();
    return store->getBool(key);
}
void prefs_set_bool(prefs_key_t key, bool val) {
    init_store_if_needed();
    ESP_LOGV(LOG_TAG, "Set %s = %s", key, val ? "TRUE" : "FALSE");
    store->putBool(key, val);
}

void prefs_set_data(prefs_key_t key, const void * data, size_t size) {
    init_store_if_needed();
    ESP_LOGV(LOG_TAG, "Set %s = (%i bytes)", key, size);
    store->putBytes(key, data, size);
}

bool prefs_get_data(prefs_key_t key, void * destination, size_t capacity, size_t * fetchedSize) {
    init_store_if_needed();
    size_t expected = store->getBytesLength(key);
    if(capacity < expected) {
        ESP_LOGE(LOG_TAG, "Get %s: wanted %i bytes, had only %i", key, expected, capacity);
        if(fetchedSize) *fetchedSize = 0;
        return false;
    }

    size_t real = store->getBytes(key, destination, capacity);
    if(fetchedSize) *fetchedSize = real;
    return (real == expected);
}