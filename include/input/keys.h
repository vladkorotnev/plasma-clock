#pragma once
#include <stdint.h>

typedef enum key_id {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HEADPAT,

    KEY_MAX_INVALID
} key_id_t;

typedef enum key_state {
    KEYSTATE_RELEASED,
    KEYSTATE_PRESSED,
    KEYSTATE_HOLDING
} key_state_t;

#define KEY_ID_TO_BIT(k) (1 << k)

typedef uint8_t key_bitmask_t;

const key_bitmask_t KEYMASK_ALL = 0xFF;

void hid_set_key_state(key_id_t key, bool state);
key_state_t hid_test_key_state(key_id_t key);
key_state_t hid_test_key_all(key_bitmask_t keys);
key_state_t hid_test_key_any(key_bitmask_t keys = KEYMASK_ALL);
