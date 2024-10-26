#include <device_config.h>

#if HAS(OUTPUT_GU7000)
#include "display/gu7000.h"
#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include <esp_err.h>

static char LOG_TAG[] = "GU7000";
static portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

ItronGU7000Driver::ItronGU7000Driver(
    const gpio_num_t databus[8],
    const gpio_num_t wr,
    const gpio_num_t busy
) {
    for(int i = 0; i < 8; i++) {
        databus_gpios[i] = databus[i];
    }
    wr_gpio = wr;
    busy_gpio = busy;
}

void ItronGU7000Driver::initialize() {
    // TODO use parlio?
    ESP_LOGI(LOG_TAG, "Initializing Noritake Itron GU7000 with data bus: %i %i %i %i %i %i %i %i, wr=%i, busy=%i", databus_gpios[0], databus_gpios[1], databus_gpios[2], databus_gpios[3], databus_gpios[4], databus_gpios[5], databus_gpios[6], databus_gpios[7], wr_gpio, busy_gpio);

    // Set up parallel output

    gpio_config_t io_conf = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE,
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << wr_gpio;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(wr_gpio, 1);

    // Set up busy input

    io_conf = {
        .pin_bit_mask = (1ULL << busy_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
        .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    wait_shit_through();
    delay(2000);
}

void ItronGU7000Driver::wait_shit_through() {
    while(gpio_get_level(busy_gpio)) {
        delayMicroseconds(2);
    }
}

void ItronGU7000Driver::set_databus(uint8_t data) {
    uint8_t local_sts = data;
    for(int i = 0; i < 8; i++) {
        uint8_t cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void ItronGU7000Driver::pulse_clock() {
    gpio_set_level(wr_gpio, 0);
    delayMicroseconds(1);
    gpio_set_level(wr_gpio, 1);
    delayMicroseconds(1);
    wait_shit_through();
}

void ItronGU7000Driver::write_string(const char * s) {
    taskENTER_CRITICAL(&_spinlock);

    int len = strlen(s);
    for(int i = 0; i < len; i++) {
        set_databus(s[i]);
        pulse_clock();
    }

    taskEXIT_CRITICAL(&_spinlock);
}

void ItronGU7000Driver::reset() {
    // taskENTER_CRITICAL(&_spinlock);
    wait_shit_through();

    // Bringup Step 1: Initialize display
    write_string("\x1B\x40");

    // Power on
    set_power(true);

    // Cursor OFF (oof the Cstring hits again!)
    write_string("\x1F\x43"); set_databus(0); pulse_clock();

    clear(); 
    write_string("\x1F\x58\x01");
    write_string("uPIS-OS Noritake GU7000 Init");

    vTaskDelay(pdMS_TO_TICKS(1000));

    clear();
    write_string("\x1F\x24"); // cursor move to 0
    set_databus(0);
    pulse_clock(); pulse_clock(); pulse_clock(); pulse_clock();
}

void ItronGU7000Driver::set_show(bool show) {

}

void ItronGU7000Driver::set_power(bool power) {
    taskENTER_CRITICAL(&_spinlock);

    write_string("\x1F\x28\x61\x40");
    set_databus(power ? 0x1 : 0x0);
    pulse_clock();

    taskEXIT_CRITICAL(&_spinlock);
}

void ItronGU7000Driver::clear() {
    taskENTER_CRITICAL(&_spinlock);
    set_databus(0x0c);
    pulse_clock();
    taskEXIT_CRITICAL(&_spinlock);
}

inline uint8_t flipByte(uint8_t c){
  char r=0;
  for(uint8_t i = 0; i < 8; i++){
    r <<= 1;
    r |= c & 1;
    c >>= 1;
  }
  return r;
}

void ItronGU7000Driver::write_fanta(const uint8_t * strides, size_t count) {
    taskENTER_CRITICAL(&_spinlock);

    write_string("\x1F\x24"); // cursor move to 0
    set_databus(0);
    pulse_clock(); pulse_clock(); pulse_clock(); pulse_clock();

    write_string("\x1F\x28\x66\x11");
    set_databus(HWCONF_DISPLAY_WIDTH_PX & 0x00FF);
    pulse_clock();
    set_databus(HWCONF_DISPLAY_WIDTH_PX & 0xFF00);
    pulse_clock();
    set_databus((HWCONF_DISPLAY_HEIGHT_PX / 8) & 0x00FF);
    pulse_clock();
    set_databus((HWCONF_DISPLAY_HEIGHT_PX / 8) & 0xFF00);
    pulse_clock();
    set_databus(1);
    pulse_clock();

    for(int i = 0; i < count; i++) {
        set_databus(flipByte(strides[i]));
        pulse_clock();
    }

    taskEXIT_CRITICAL(&_spinlock);
}

void ItronGU7000Driver::set_bright(bool bright) {
    taskENTER_CRITICAL(&_spinlock);
    
    char brightness = bright ? 6 : 2; // range 1..8, 25% and 75% in this case
    write_string("\x1F\x58");
    set_databus(brightness); pulse_clock();

    taskEXIT_CRITICAL(&_spinlock);
}

#endif