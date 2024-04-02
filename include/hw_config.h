#pragma once

#include <hal/gpio_hal.h>

// ---- Connection to DISP BOARD ----

// Oh well, I hoped my ESP32 supports Dedicated GPIO but no!
const gpio_num_t HWCONF_PLASMA_DATABUS_GPIOS[] = {
    GPIO_NUM_15, 
    GPIO_NUM_2, 
    GPIO_NUM_0,
    GPIO_NUM_4, 
    GPIO_NUM_16, 
    GPIO_NUM_17, 
    GPIO_NUM_5, 
    GPIO_NUM_18
};
const gpio_num_t HWCONF_PLASMA_CLK_GPIO = GPIO_NUM_19;
const gpio_num_t HWCONF_PLASMA_RESET_GPIO = GPIO_NUM_21;
const gpio_num_t HWCONF_PLASMA_BRIGHT_GPIO = GPIO_NUM_22;
const gpio_num_t HWCONF_PLASMA_SHOW_GPIO = GPIO_NUM_23;

// ---- Connection to HV Board ----
const gpio_num_t HWCONF_PLASMA_HV_EN_GPIO = GPIO_NUM_13;

// ---- Connection to LV Board (or beeper directly) ----
const gpio_num_t HWCONF_BEEPER_GPIO = GPIO_NUM_12;

// ---- Connection & setting of EYE BOARD ----
const gpio_num_t HWCONF_MOTION_GPIO = GPIO_NUM_14;
const gpio_num_t HWCONF_LIGHTSENSE_GPIO = GPIO_NUM_27;
const unsigned int HWCONF_LIGHTSENSE_MINIMUM = 0;
const unsigned int HWCONF_LIGHTSENSE_MAXIMUM = 4096;

// ---- Connection of NOSE BOARD ----
const gpio_num_t HWCONF_I2C_SDA_GPIO = GPIO_NUM_26;
const gpio_num_t HWCONF_I2C_SCL_GPIO = GPIO_NUM_25;
