#include <input/keys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

const TickType_t KEYPRESS_THRESHOLD_TIME = pdMS_TO_TICKS(16);
const TickType_t KEYHOLD_THRESHOLD_TIME = pdMS_TO_TICKS(1000);

static key_bitmask_t active_keys = 0;
static TickType_t keypress_started_at[KEY_MAX_INVALID] = { 0 };

inline key_state_t time_to_state(TickType_t time) {
    if(time < KEYPRESS_THRESHOLD_TIME) return KEYSTATE_RELEASED;
    if(time < KEYHOLD_THRESHOLD_TIME) return KEYSTATE_PRESSED;
    
    return KEYSTATE_HOLDING;
}

static key_state_t min_state_of_mask(key_bitmask_t keys) {
    TickType_t min = portMAX_DELAY;
    for(key_id_t i = (key_id_t)0; i < KEY_MAX_INVALID; i = (key_id_t) (i + 1)) {
        if((keys & KEY_ID_TO_BIT(i)) == 0) continue;

        if(min > keypress_started_at[i]) {
            min = keypress_started_at[i];
        }
    }

    return time_to_state(min);
}

void hid_set_key_state(key_id_t key, bool state) {
    if(state) {
        keypress_started_at[key] = xTaskGetTickCount();
        active_keys |= KEY_ID_TO_BIT(key);
    } else {
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