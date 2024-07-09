#pragma once
#include <device_config.h>
#include <freertos/FreeRTOS.h>

void wotd_start();
bool wotd_get_current(char * word, size_t word_sz, char * definition, size_t def_sz);
TickType_t wotd_get_last_update();