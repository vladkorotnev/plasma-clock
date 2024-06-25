#pragma once

#include <hal/gpio_hal.h>

#define HAS_OUTPUT_WS0010
#define HAS_TEMP_SENSOR

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "microPIS-OS"
#define PRODUCT_VERSION "1.0"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_12;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection to temperature sensor ----
const gpio_num_t HWCONF_I2C_SDA_GPIO = GPIO_NUM_26;
const gpio_num_t HWCONF_I2C_SCL_GPIO = GPIO_NUM_25;
