#include <device_config.h>

#if HAS(OUTPUT_WS0010)
#include "display/ws0010.h"
#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include <esp_err.h>

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
    ddram_ptr = 0;

    initialize();
}

void Ws0010OledDriver::initialize() {
    ESP_LOGI(LOG_TAG, "Initializing Winstar WS0010 OLED bus with data bus: %i %i %i %i %i %i %i %i, rs=%i, en=%i, bright=%i, show=%i, hv_en=%i", databus_gpios[0], databus_gpios[1], databus_gpios[2], databus_gpios[3], databus_gpios[4], databus_gpios[5], databus_gpios[6], databus_gpios[7], rs_gpio, en_gpio);

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << rs_gpio;
    io_conf.pin_bit_mask |= 1ULL << en_gpio;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    delay(500); // as per datasheet
    reset();
}

void Ws0010OledDriver::set_databus(uint8_t data) {
    uint8_t local_sts = ~data;
    for(int i = 0; i < 8; i++) {
        uint8_t cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void Ws0010OledDriver::pulse_clock() {
    gpio_set_level(en_gpio, 0);
    delayMicroseconds(10);
    gpio_set_level(en_gpio, 1);
    delayMicroseconds(10);
}

void Ws0010OledDriver::set_is_command(bool rs) {
    if(rs) {
        // any command will interrupt the writing of graphic data
        // thus clear respective flags
        is_writing_ddram = false;
        ddram_ptr = 0;
    }
    gpio_set_level(rs_gpio, rs ? 0 : 1);
}

void Ws0010OledDriver::reset() {
    taskENTER_CRITICAL(&_spinlock);

    // Bringup step 1: Function set
    set_is_command(true);
    set_databus(0b00111100); // Function Set, DB4=1 (8bit), DB3=1 (Nlines=2), DB2=0 (Font=0), DB1=0 DB0=0 (Font Table = 0)
    pulse_clock();
    delayMicroseconds(400);

    // Step 2: OnOff Control
    set_show(true);

    // Step 3: Clear
    clear();

    // Step 4: Entry mode set
    set_is_command(true);
    set_databus(0b00000110); // autoincrement mode, shift disable
    pulse_clock();
    delayMicroseconds(400);

    // Step 5: Set graphic mode
    set_power(true);

    taskEXIT_CRITICAL(&_spinlock);
}

void Ws0010OledDriver::set_show(bool show) {
    set_is_command(true);
    set_databus(0b00001000 | (show ? 0b100 : 0)); // cursor and blink always off
    pulse_clock();
    delayMicroseconds(400);
}

void Ws0010OledDriver::set_power(bool power) {
    set_is_command(true);
    set_databus(0b00011000 | (power ? 0b100 : 0)); // G/C always set to GRAPHIC
    pulse_clock();
    delayMicroseconds(400);
}

void Ws0010OledDriver::clear() {
    set_is_command(true);
    set_databus(0b00000001);
    pulse_clock();
    delayMicroseconds(400);
}

void Ws0010OledDriver::write_stride(uint8_t stride) {
    if(ddram_ptr <= 199) {
        taskENTER_CRITICAL(&_spinlock);
        // someday, properly support various screen sizes
        // for now just don't send the last column

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

        taskEXIT_CRITICAL(&_spinlock);
    }
    ddram_ptr++;
    if(ddram_ptr == 202) {
        ddram_ptr = 0;
    }
}

#endif