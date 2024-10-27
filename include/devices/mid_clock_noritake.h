#pragma once

#include <hal/gpio_hal.h>
#include <input/keypad.h>

#define HAS_OTAFVU
#define HAS_OUTPUT_GU7000
#define HAS_MOTION_SENSOR
#define HAS_TEMP_SENSOR
#define HAS_KEYPAD
#define HAS_SERIAL_MIDI
#define HAS_VARYING_BRIGHTNESS
#define HAS_LIGHT_SENSOR

//      ↓ Looks like shite on the VFD display
#define COMPOSABLE_NO_EVENODD

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "PIS-OS"
#define PRODUCT_VERSION "5"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_33;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection to temperature sensor ----
const gpio_num_t HWCONF_I2C_SDA_GPIO = GPIO_NUM_26;
const gpio_num_t HWCONF_I2C_SCL_GPIO = GPIO_NUM_25;

// ---- Connection to light sensors ----
const gpio_num_t HWCONF_LIGHTSENSE_GPIO = GPIO_NUM_36;
const gpio_num_t HWCONF_MOTION_GPIO = GPIO_NUM_39;

// ---- Connection to display ----
const gpio_num_t HWCONF_GU7000_DATABUS_GPIOS[] = {
    GPIO_NUM_5,  
    GPIO_NUM_0,  
    GPIO_NUM_23,
    GPIO_NUM_2,  
    GPIO_NUM_22,
    GPIO_NUM_15,
    GPIO_NUM_4, 
    GPIO_NUM_21 
};
const gpio_num_t HWCONF_GU7000_WR_GPIO = GPIO_NUM_18;
const gpio_num_t HWCONF_GU7000_BUSY_GPIO = GPIO_NUM_19;

#define HWCONF_DISPLAY_WIDTH_PX 112
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection of buttons ----
const keypad_definition_t HWCONF_KEYPAD = {
    {GPIO_NUM_14, KEY_RIGHT},
    {GPIO_NUM_32, KEY_LEFT},
    {GPIO_NUM_34, KEY_UP},
    {GPIO_NUM_27, KEY_DOWN},
    {GPIO_NUM_35, KEY_HEADPAT}, //<- finally a clock to have hardware headpat!
};
