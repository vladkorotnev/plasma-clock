#pragma once

#include <hal/gpio_hal.h>
#include <input/keypad.h>

#define HAS_OUTPUT_GU312
#define HAS_KEYPAD

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "LongPIS-OS"
#define PRODUCT_VERSION "5.3"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_33;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection to display ----
const gpio_num_t HWCONF_GU312_DATABUS_GPIOS[] = {
    GPIO_NUM_5,  
    GPIO_NUM_0,  
    GPIO_NUM_23,
    GPIO_NUM_2,  
    GPIO_NUM_22,
    GPIO_NUM_15,
    GPIO_NUM_4, 
    GPIO_NUM_21 
};
const gpio_num_t HWCONF_GU312_CD_GPIO = GPIO_NUM_19; 
const gpio_num_t HWCONF_GU312_WR_GPIO = GPIO_NUM_18;

#define HWCONF_DISPLAY_WIDTH_PX 192
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection of buttons ----
const keypad_definition_t HWCONF_KEYPAD = {
    {GPIO_NUM_14, KEY_RIGHT},
    {GPIO_NUM_32, KEY_LEFT},
    {GPIO_NUM_34, KEY_UP},
    {GPIO_NUM_27, KEY_DOWN},
    {GPIO_NUM_35, KEY_HEADPAT},
};
