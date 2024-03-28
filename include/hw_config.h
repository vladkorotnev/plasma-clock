#pragma once

#include <hal/gpio_hal.h>

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
const gpio_num_t HWCONF_PLASMA_RTZ_GPIO = GPIO_NUM_13; // not used
const gpio_num_t HWCONF_PLASMA_RESET_GPIO = GPIO_NUM_21;
const gpio_num_t HWCONF_PLASMA_BRIGHT_GPIO = GPIO_NUM_22;
const gpio_num_t HWCONF_PLASMA_SHOW_GPIO = GPIO_NUM_23;
