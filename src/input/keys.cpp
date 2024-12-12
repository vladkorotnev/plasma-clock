#include <input/keys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp32-hal-log.h>
#include <unordered_set>

const TickType_t KEYPRESS_THRESHOLD_TIME = pdMS_TO_TICKS(16);
const TickType_t KEYHOLD_THRESHOLD_TIME = pdMS_TO_TICKS(1000);
const TickType_t KEYHOLD_REPETITION_TIME = pdMS_TO_TICKS(500);
const TickType_t KEYHOLD_REPETITION_SPEEDUP_TIME = pdMS_TO_TICKS(3000);

static key_bitmask_t active_keys = 0;
static std::unordered_set<key_bitmask_t> pressed_keycombos = {};
static TickType_t keypress_started_at[KEY_MAX_INVALID] = { 0 };
static TickType_t keypress_repeated_at[KEY_MAX_INVALID] = { 0 };
static Beeper * beepola = nullptr;
static TimerSensor * hid_state_sensor = nullptr;
static bool hid_locked = false;

void hid_set_lock_state(bool locked) {
    hid_locked = locked;
}

void hid_set_key_beeper(Beeper* b) {
    beepola = b;
}

inline key_state_t time_to_state(TickType_t time) {
    TickType_t now = xTaskGetTickCount();

    if((now - time) < KEYPRESS_THRESHOLD_TIME) return KEYSTATE_RELEASED;
    if((now - time) < KEYHOLD_THRESHOLD_TIME) return KEYSTATE_PRESSED;
    
    return KEYSTATE_HOLDING;
}

static key_state_t min_state_of_mask(key_bitmask_t keys, bool peek = false) {
    if(hid_locked) return KEYSTATE_RELEASED;
    
    TickType_t maxTimeStamp = 0;
    for(key_id_t i = (key_id_t)0; i < KEY_MAX_INVALID; i = (key_id_t) (i + 1)) {
        if((keys & KEY_ID_TO_BIT(i)) == 0) continue;

        if(maxTimeStamp < keypress_started_at[i]) {
            maxTimeStamp = keypress_started_at[i];
        }
    }

    key_state_t preliminary_state = time_to_state(maxTimeStamp);

    switch(preliminary_state) {
        case KEYSTATE_RELEASED:
            pressed_keycombos.erase(keys);
            return KEYSTATE_RELEASED;

        case KEYSTATE_PRESSED:
            if(pressed_keycombos.count(keys) == 0) {
                if(!peek) {
                    pressed_keycombos.insert(keys);
                    if(beepola != nullptr) beepola->beep(CHANNEL_NOTICE, 1000, 10);
                }
                return KEYSTATE_HIT;
            } else {
                return KEYSTATE_PRESSED;
            }

        default: return preliminary_state;
    }
}

void hid_set_key_state(key_id_t key, bool state) {
    if(state && (active_keys & KEY_ID_TO_BIT(key)) == 0) {
        keypress_started_at[key] = xTaskGetTickCount();
        keypress_repeated_at[key] = xTaskGetTickCount();
        active_keys |= KEY_ID_TO_BIT(key);
        if(hid_state_sensor) hid_state_sensor->trigger();
    } else if(!state && (active_keys & KEY_ID_TO_BIT(key)) != 0) {
        active_keys &= ~KEY_ID_TO_BIT(key);
    }
}

key_state_t hid_test_key_state(key_id_t key) {
    return hid_test_key_all(KEY_ID_TO_BIT(key));
}

key_state_t hid_test_key_all(key_bitmask_t keys) {
    if((active_keys & keys) != keys) return KEYSTATE_RELEASED;
    return min_state_of_mask(keys);
}

key_state_t hid_test_key_any(key_bitmask_t keys) {
    if((active_keys & keys) == 0) return KEYSTATE_RELEASED;
    return min_state_of_mask((active_keys & keys));
}

key_state_t hid_peek_key_any(key_bitmask_t keys) {
    if((active_keys & keys) == 0) return KEYSTATE_RELEASED;
    return min_state_of_mask((active_keys & keys), true);
}

key_state_t hid_test_key_state_repetition(key_id_t key) {
    switch(hid_test_key_state(key)) {
        case KEYSTATE_RELEASED:
        case KEYSTATE_PRESSED: 
            return KEYSTATE_RELEASED;
        case KEYSTATE_HIT:
            return KEYSTATE_HIT;
        case KEYSTATE_HOLDING:
            {
                TickType_t now = xTaskGetTickCount();
                TickType_t repeat_delay = (now - keypress_started_at[key] >= KEYHOLD_REPETITION_SPEEDUP_TIME) ? (KEYHOLD_REPETITION_TIME / 2) : KEYHOLD_REPETITION_TIME;
                if(now - keypress_repeated_at[key] >= repeat_delay) {
                    keypress_repeated_at[key] = now;
                    return KEYSTATE_HIT;
                } else {
                    return KEYSTATE_RELEASED;
                }
            }
            break;
        default: return KEYSTATE_RELEASED;
    }
}


PoolableSensor * hid_get_state_sensor() {
    if(hid_state_sensor == nullptr) {
        hid_state_sensor = new TimerSensor(pdMS_TO_TICKS(3000));
    }

    return hid_state_sensor;
}