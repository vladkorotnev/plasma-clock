#pragma once

#include <hal/gpio_hal.h>
#include <input/keypad.h>
#include <input/infrared.h>

#define HAS_OUTPUT_GU312
#define HAS_KEYPAD
#define HAS_VARYING_BRIGHTNESS
#define HAS_MOTION_SENSOR
#define HAS_IR_RECEIVER

// Plasma Information System OS (not DOS, there's no disk in it!)
#define PRODUCT_NAME "LongPIS-OS"
#define PRODUCT_VERSION "5.3"

// ---- Connection to beeper ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_33;
const uint8_t HWCONF_BEEPER_PWM_CHANNEL = 0;

// ---- Connection to light sensors ----
const gpio_num_t HWCONF_MOTION_GPIO = GPIO_NUM_39;
const gpio_num_t HWCONF_LIGHTSENSE_GPIO = GPIO_NUM_36;
const gpio_num_t HWCONF_IR_RECV_GPIO = GPIO_NUM_14;

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
const gpio_num_t HWCONF_GU312_BLANKING_GPIO = GPIO_NUM_12;
const gpio_num_t HWCONF_GU312_FEP_GPIO = GPIO_NUM_13; //<- optional, can be GPIO_NUM_NC

#define HWCONF_DISPLAY_WIDTH_PX 192
#define HWCONF_DISPLAY_HEIGHT_PX 16

// ---- Connection of buttons ----
const keypad_definition_t HWCONF_KEYPAD = {
    {GPIO_NUM_27, KEY_RIGHT},
    {GPIO_NUM_32, KEY_LEFT},
    {GPIO_NUM_34, KEY_UP},
    {GPIO_NUM_35, KEY_DOWN},
};

const infrared_definition_t HWCONF_IR_BUTTONS = {
    /**
        Optosupply OE13KIR (https://akizukidenshi.com/goodsaffix/OE13KIR.pdf)
    */
    { .protocol = IRPROTO_NEC, .address = 0x10, .command = 0xA0, .value = 0x8F705FA, .key = KEY_UP}, // UP
    { .protocol = IRPROTO_NEC, .address = 0x10, .command = 0x00, .value = 0x8F700FF, .key = KEY_DOWN}, // DOWN
    { .protocol = IRPROTO_NEC, .address = 0x10, .command = 0x10, .value = 0x8F708F7, .key = KEY_LEFT}, // LEFT
    { .protocol = IRPROTO_NEC, .address = 0x10, .command = 0x80, .value = 0x8F701FE, .key = KEY_RIGHT}, // RIGHT
    { .protocol = IRPROTO_NEC, .address = 0x10, .command = 0x20, .value = 0x8F704FB, .key = KEY_HEADPAT}, // CENTER
    // Top-left: TYPE=3, VALUE=0x8f78d72, ADDRESS=0x10, COMMAND=0xb1
    // Top-right: TYPE=3, VALUE=0x8f7847b, ADDRESS=0x10, COMMAND=0x21
    // Bottom-left: TYPE=3, VALUE=0x8f78877, ADDRESS=0x10, COMMAND=0x11
    // Bottom-right: TYPE=3, VALUE=0x8f7817e, ADDRESS=0x10, COMMAND=0x81
    // Power: TYPE=3, VALUE=0x8f71be4, ADDRESS=0x10, COMMAND=0xd8
    // A: TYPE=3, VALUE=0x8f71fe0, ADDRESS=0x10, COMMAND=0xf8
    // B: TYPE=3, VALUE=0x8f71ee1, ADDRESS=0x10, COMMAND=0x78
    // C: TYPE=3, VALUE=0x8f71ae5, ADDRESS=0x10, COMMAND=0x58
};