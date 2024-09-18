#pragma once

#include <hal/gpio_hal.h>
#include <input/keypad.h>

#define HAS_OUTPUT_MD_PLASMA
#define HAS_VARYING_BRIGHTNESS
#define HAS_LIGHT_SENSOR
#define HAS_TEMP_SENSOR
#define HAS_MOTION_SENSOR
#define HAS_KEYPAD

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "PIS-OS"
#define PRODUCT_VERSION "3.8"

// ---- Connection to DISP BOARD ----
const gpio_num_t HWCONF_PLASMA_DATABUS_GPIOS[] = {
    GPIO_NUM_15, 
    GPIO_NUM_2, 
    GPIO_NUM_0,
    GPIO_NUM_4, 
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_5, 
    GPIO_NUM_18
};
const gpio_num_t HWCONF_PLASMA_CLK_GPIO = GPIO_NUM_19;
const gpio_num_t HWCONF_PLASMA_RESET_GPIO = GPIO_NUM_21;
const gpio_num_t HWCONF_PLASMA_BRIGHT_GPIO = GPIO_NUM_22;
const gpio_num_t HWCONF_PLASMA_SHOW_GPIO = GPIO_NUM_23;

#define HWCONF_DISPLAY_WIDTH_PX 101
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection to HV Board ----
const gpio_num_t HWCONF_PLASMA_HV_EN_GPIO = GPIO_NUM_13;

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_12;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection & setting of EYE BOARD ----
const gpio_num_t HWCONF_MOTION_GPIO = GPIO_NUM_14;
const gpio_num_t HWCONF_LIGHTSENSE_GPIO = GPIO_NUM_35;

// ---- Connection of NOSE BOARD ----
const gpio_num_t HWCONF_I2C_SDA_GPIO = GPIO_NUM_26;
const gpio_num_t HWCONF_I2C_SCL_GPIO = GPIO_NUM_25;

// ---- Connection of buttons ----
const keypad_definition_t HWCONF_KEYPAD = {
    {GPIO_NUM_27, KEY_LEFT},
    {GPIO_NUM_39, KEY_RIGHT},
    {GPIO_NUM_34, KEY_UP},
    {GPIO_NUM_36, KEY_DOWN},
};
