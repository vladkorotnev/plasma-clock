#pragma once
#include "keys.h"
#include <vector>
#include "esp32-hal-gpio.h"

typedef std::vector<std::pair<const gpio_num_t, const key_id_t>> keypad_definition_t;
void keypad_start();
