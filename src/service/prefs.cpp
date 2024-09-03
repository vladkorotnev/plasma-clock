#include "service/prefs.h"
#include <Preferences.h>
#include "nvs_flash.h"

static char LOG_TAG[] = "PREF";
static Preferences * store = nullptr;
static constexpr const char * STORE_DOMAIN = "pisos_prefs";

inline void init_store_if_needed() {
    if(store == nullptr) {
        ESP_LOGI(LOG_TAG, "Initialize");
        store = new Preferences();
        nvs_flash_init(); // <- sometimes language settings are being checked before Arduino finished initializing, so we need to bring NVS up on our own
        if(!store->begin(STORE_DOMAIN)) {
            store = nullptr;
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