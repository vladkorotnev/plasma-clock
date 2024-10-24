#pragma once

#include <hal/gpio_hal.h>
#include <input/keypad.h>

// Clock made around the PCB designed in 2024/09 labeled
// 'uPIS-OS Devkit / Genjitsu Labs 2024'
// 'microPIS-OS CPU BD V1.0 2024-09-16'

#define HAS_OUTPUT_WS0010
#define HAS_TEMP_SENSOR
#define HAS_KEYPAD
#define HAS_VARYING_BRIGHTNESS
#define HAS_LIGHT_SENSOR
#define HAS_MOTION_SENSOR

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "uPIS-OS"
#define PRODUCT_VERSION "4"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_33;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection to temperature sensor ----
const gpio_num_t HWCONF_I2C_SDA_GPIO = GPIO_NUM_26;
const gpio_num_t HWCONF_I2C_SCL_GPIO = GPIO_NUM_25;

// ---- Connection to display ----
const gpio_num_t HWCONF_WS0010_DATABUS_GPIOS[] = {
    GPIO_NUM_5,  
    GPIO_NUM_0,  
    GPIO_NUM_23,
    GPIO_NUM_2,  
    GPIO_NUM_22,
    GPIO_NUM_15,
    GPIO_NUM_4, 
    GPIO_NUM_21 
};
const gpio_num_t HWCONF_WS0010_RS_GPIO = GPIO_NUM_19; 
const gpio_num_t HWCONF_WS0010_EN_GPIO = GPIO_NUM_18;

// ---- Connection to light sensors ----
const gpio_num_t HWCONF_MOTION_GPIO = GPIO_NUM_39;
const gpio_num_t HWCONF_LIGHTSENSE_GPIO = GPIO_NUM_36;

#define HWCONF_DISPLAY_WIDTH_PX 100
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection of buttons ----
const keypad_definition_t HWCONF_KEYPAD = {
    {GPIO_NUM_14, KEY_RIGHT},
    {GPIO_NUM_32, KEY_LEFT},
    {GPIO_NUM_34, KEY_UP},
    {GPIO_NUM_27, KEY_DOWN},
    {GPIO_NUM_35, KEY_HEADPAT}, //<- finally a clock to have hardware headpat!
};
