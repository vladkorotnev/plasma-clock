#include "service/prefs.h"
#include <Preferences.h>

static char LOG_TAG[] = "PREF";
static Preferences * store = nullptr;
static constexpr const char * STORE_DOMAIN = "pisos_prefs";

inline void init_store_if_needed() {
    if(store == nullptr) {
        ESP_LOGI(LOG_TAG, "Initialize");
        store = new Preferences();
        store->begin(STORE_DOMAIN);
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