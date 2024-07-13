#pragma once

#include <hal/gpio_hal.h>
#include <input/touch_plane.h>

#define HAS_OUTPUT_WS0010
#define HAS_TEMP_SENSOR
#define HAS_TOUCH_PLANE

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "microPIS-OS"
#define PRODUCT_VERSION "1.0"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_12;
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

#define HWCONF_DISPLAY_WIDTH_PX 100
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection to touch plane ----
const touch_plane_definition_t HWCONF_TOUCH_PLANE = {
    // Screen panel
    {/*GPIO_NUM_32*/ TOUCH_PAD_NUM9, {.key = KEY_RIGHT, .press_threshold = 7, .release_threshold = -3}},
    {/*GPIO_NUM_33*/ TOUCH_PAD_NUM8, {.key = KEY_UP, .press_threshold = 7, .release_threshold = -2}},
    {/*GPIO_NUM_27*/ TOUCH_PAD_NUM7, {.key = KEY_DOWN, .press_threshold = 7, .release_threshold = -2}},
    {/*GPIO_NUM_14*/ TOUCH_PAD_NUM6, {.key = KEY_LEFT, .press_threshold = 7, .release_threshold = -2}},
    // Top of case
    // {/*GPIO_NUM_13*/ TOUCH_PAD_NUM4, KEY_HEADPAT},
};