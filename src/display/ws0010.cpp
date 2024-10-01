#include <device_config.h>

#if HAS(OUTPUT_WS0010)
#include "display/ws0010.h"
#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include <esp_err.h>

#ifndef WS0010_BFI_DIMMER_DURATION_US
#define WS0010_BFI_DIMMER_DURATION_US 6000 //<- experimentally found value for roughly 50% brightness @ 3V3 supply
#endif

static char LOG_TAG[] = "Winstar0010";
static portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

Ws0010OledDriver::Ws0010OledDriver(
    const gpio_num_t databus[8],
    const gpio_num_t rs,
    const gpio_num_t en
) {
    for(int i = 0; i < 8; i++) {
        databus_gpios[i] = databus[i];
    }
    rs_gpio = rs;
    en_gpio = en;
    is_writing_ddram = false;
    is_dim = false;
    ddram_ptr = 0;
}

void Ws0010OledDriver::initialize() {
    // TODO use parlio?
    ESP_LOGI(LOG_TAG, "Initializing Winstar WS0010 OLED bus with data bus: %i %i %i %i %i %i %i %i, rs=%i, en=%i", databus_gpios[0], databus_gpios[1], databus_gpios[2], databus_gpios[3], databus_gpios[4], databus_gpios[5], databus_gpios[6], databus_gpios[7], rs_gpio, en_gpio);

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE,
        //.pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_ENABLE
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << rs_gpio;
    io_conf.pin_bit_mask |= 1ULL << en_gpio;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(en_gpio, 0);
    gpio_set_level(rs_gpio, 1);

    delay(1500); // as per datasheet
}

void Ws0010OledDriver::set_databus(uint8_t data) {
    uint8_t local_sts = data;
    for(int i = 0; i < 8; i++) {
        uint8_t cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void Ws0010OledDriver::pulse_clock() {
    gpio_set_level(en_gpio, 1);
    delayMicroseconds(1);
    gpio_set_level(en_gpio, 0);
}

void Ws0010OledDriver::set_is_command(bool rs) {
    if(rs) {
        // any command will interrupt the writing of graphic data
        // thus clear respective flags
        is_writing_ddram = false;
        ddram_ptr = 0;
    }
    // H: DATA, L: Instruction code
    gpio_set_level(rs_gpio, rs ? 0 : 1);
    delayMicroseconds(1);
}

void Ws0010OledDriver::write_string(const char * s) {
    set_is_command(false);
    int len = strlen(s);
    for(int i = 0; i < len; i++) {
        set_databus(s[i]);
        pulse_clock();
        delayMicroseconds(100);
    }
}

void Ws0010OledDriver::reset() {
    taskENTER_CRITICAL(&_spinlock);

    // Bringup step 1: Function set
    set_is_command(true);
    set_databus(0b00111000); // Function Set, DB4=1 (8bit), DB3=0 (Nlines=2), DB2=0 (Font=0), DB1=0 DB0=0 (Font Table = 0)
    pulse_clock();

    // Step 2: OnOff Control
    set_show(true);

    // Step 3: Clear
    clear();
    delay(500);

    // Step 4: Entry mode set
    set_is_command(true);
    set_databus(0b00000110); // autoincrement mode, shift disable
    pulse_clock();
    delayMicroseconds(400);

    set_is_command(true);
    set_databus(0b10); // return home
    pulse_clock();
    delayMicroseconds(400);
    delay(200);

    write_string("uPIS-OS init");
    delay(500);

    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::set_show(bool show) {
    taskENTER_CRITICAL(&_spinlock);
    show_state = show;
    set_is_command(true);
    set_databus(0b00001000 | (show ? 0b111 : 0)); // cursor and blink always off
    pulse_clock();
    delayMicroseconds(10);
    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::set_power(bool power) {
    taskENTER_CRITICAL(&_spinlock);
    set_is_command(true);
    set_databus(0b00011000 | (power ? 0b111 : 0b011)); // G/C always set to GRAPHIC
    pulse_clock();
    delayMicroseconds(400);
    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::clear() {
    taskENTER_CRITICAL(&_spinlock);
    set_is_command(true);
    set_databus(0b00000001);
    pulse_clock();
    delayMicroseconds(400);
    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::write_stride(uint8_t stride) {
    if(!is_writing_ddram) {
        // set ddram command to synchronize the pointer
        set_is_command(true);
        set_databus(0b10000000 | (ddram_ptr & 0b01111111));
        pulse_clock();
        is_writing_ddram = true;
        set_is_command(false);
    }

    set_databus(stride);
    pulse_clock();
}

void Ws0010OledDriver::write_fanta(const uint8_t * strides, size_t count) {
    taskENTER_CRITICAL(&_spinlock);
#ifndef WS0010_NO_BFI
    bool show_backup = show_state;
    set_show(false);
#endif
    // First write even (top row), then odd (bottom row)
    for(int i = 0; i < count - 1; i += 2) {
        if(i >= 200) continue;
        write_stride(strides[i]);
    }
    for(int i = 1; i < count; i += 2) {
        if(i >= 200) continue;
        write_stride(strides[i]);
    }
#ifndef WS0010_NO_BFI
    if(is_dim) {
        delayMicroseconds(WS0010_BFI_DIMMER_DURATION_US);
    }
    set_show(show_backup);
#endif
    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::set_bright(bool bright) {
    ESP_LOGI(LOG_TAG, "BFI dimming is now %s", bright ? "OFF":"ON");
    is_dim = !bright;
}

#endif