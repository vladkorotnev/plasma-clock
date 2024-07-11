#pragma once
#include "keys.h"
#include <vector>
#include <esp32-hal-touch.h>

typedef std::vector<std::pair<const touch_pad_t, const key_id_t>> touch_plane_definition_t;

esp_err_t start_touchplane_scan();
