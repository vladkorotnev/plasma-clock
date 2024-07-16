#pragma once
#include "keys.h"
#include <vector>
#include <esp32-hal-touch.h>

typedef struct touchpad_mapping {
    const key_id_t key;
    const int press_threshold;
    const int release_threshold;
} touchpad_mapping_t;

typedef std::vector<std::pair<const touch_pad_t, const touchpad_mapping_t>> touch_plane_definition_t;

esp_err_t touchplane_start();
