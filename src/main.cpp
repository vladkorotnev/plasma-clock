#include <Arduino.h>
#include "driver/dedic_gpio.h"

static char LOG_TAG[] = "APL_MAIN";

// Oh well, I hoped my ESP32 supports Dedicated GPIO but no!
const gpio_num_t databus_gpios[] = {
    GPIO_NUM_15, 
    GPIO_NUM_2, 
    GPIO_NUM_0,
    GPIO_NUM_4, 
    GPIO_NUM_16, 
    GPIO_NUM_17, 
    GPIO_NUM_5, 
    GPIO_NUM_18
};
const gpio_num_t clk_gpio = GPIO_NUM_19;
const gpio_num_t rtz_gpio = GPIO_NUM_21;
const gpio_num_t bright_gpio = GPIO_NUM_22;
const gpio_num_t show_gpio = GPIO_NUM_23;

void set_databus(byte status) {
    byte local_sts = ~status;
    for(int i = 0; i < 8; i++) {
        byte cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void set_show(bool show) {
    gpio_set_level(show_gpio, show ? 1 : 0);
}

void set_bright(bool bright) {
    gpio_set_level(bright_gpio, bright ? 1 : 0);
}

void pulse_rtz() {
    gpio_set_level(rtz_gpio, 0);
    delayMicroseconds(100);
    gpio_set_level(rtz_gpio, 1);
    delay(100);
}

void init_databus() {
    ESP_LOGI(LOG_TAG, "Init databus");

    static gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << clk_gpio;
    io_conf.pin_bit_mask |= 1ULL << rtz_gpio;
    io_conf.pin_bit_mask |= 1ULL << bright_gpio;
    io_conf.pin_bit_mask |= 1ULL << show_gpio;

    gpio_config(&io_conf);

    set_databus(0);
    gpio_set_level(show_gpio, 0);
    gpio_set_level(bright_gpio, 0);
    gpio_set_level(clk_gpio, 1);
    gpio_set_level(rtz_gpio, 1);

    pulse_rtz();
}

void pulse_clk() {
    gpio_set_level(clk_gpio, 0);
    delayMicroseconds(5);
    gpio_set_level(clk_gpio, 1);
    delayMicroseconds(5);
}



void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    init_databus();

    set_show(true);
    set_bright(false);

    set_databus(0x55);
    for(int i = 0; i < 202; i++) {
        pulse_clk();
    }

   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}

uint16_t now = 1;
bool dir = true;

void loop() {
    if(now == 0x8000) dir = false;
    else if(now == 1) dir = true;

    set_databus(now & 0xFF);
    pulse_clk();
    set_databus((now >> 8) & 0xFF);
    pulse_clk();

    if(dir) now <<= 1;
    else now >>= 1;

    delay(10);
}
