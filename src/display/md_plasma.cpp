#include <device_config.h>

#if HAS(OUTPUT_MD_PLASMA)
#include <display/md_plasma.h>
#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include <esp_err.h>

static char LOG_TAG[] = "MDPlasma";

MorioDenkiPlasmaDriver::MorioDenkiPlasmaDriver(
    const gpio_num_t databus[8],
    const gpio_num_t clock,
    const gpio_num_t reset,
    const gpio_num_t bright,
    const gpio_num_t show,
    const gpio_num_t hv_enable
) {
    for(int i = 0; i < 8; i++) {
        databus_gpios[i] = databus[i];
    }

    clk_gpio = clock;
    reset_gpio = reset;
    bright_gpio = bright;
    show_gpio = show;
    hv_en_gpio = hv_enable;
}

void MorioDenkiPlasmaDriver::initialize() {
    ESP_LOGI(LOG_TAG, "Initializing MD16101DS bus with data bus: %i %i %i %i %i %i %i %i, clock=%i, reset=%i, bright=%i, show=%i, hv_en=%i", databus_gpios[0], databus_gpios[1], databus_gpios[2], databus_gpios[3], databus_gpios[4], databus_gpios[5], databus_gpios[6], databus_gpios[7], clk_gpio, reset_gpio, bright_gpio, show_gpio, hv_en_gpio);

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << clk_gpio;
    io_conf.pin_bit_mask |= 1ULL << hv_en_gpio;
    io_conf.pin_bit_mask |= 1ULL << bright_gpio;
    io_conf.pin_bit_mask |= 1ULL << show_gpio;
    io_conf.pin_bit_mask |= 1ULL << reset_gpio;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    set_databus(0x0);
    gpio_set_level(hv_en_gpio, 0);
    gpio_set_level(show_gpio, 0);
    gpio_set_level(bright_gpio, 0);
    gpio_set_level(reset_gpio, 0);
    gpio_set_level(clk_gpio, 1);

    reset();
}

void MorioDenkiPlasmaDriver::reset() {
    ESP_LOGV(LOG_TAG, "Reset");
    gpio_set_level(reset_gpio, 0);
    delayMicroseconds(100);
    gpio_set_level(reset_gpio, 1);
    // Properly we'd wait for #RDY to go high but I didn't have enough level shifters handy so CBA
    delay(100);
}

void MorioDenkiPlasmaDriver::set_power(bool on) {
    ESP_LOGV(LOG_TAG, "HIGH VOLTAGE is now %s", (on ? "ON" : "off"));
    gpio_set_level(hv_en_gpio, on ? 1 : 0);
}

void MorioDenkiPlasmaDriver::set_databus(uint8_t data) {
    uint8_t local_sts = ~data;
    for(int i = 0; i < 8; i++) {
        uint8_t cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void MorioDenkiPlasmaDriver::pulse_clock() {
    gpio_set_level(clk_gpio, 0);
    delayMicroseconds(1);
    gpio_set_level(clk_gpio, 1);
    delayMicroseconds(1);
}

void MorioDenkiPlasmaDriver::set_show(bool show) {
    ESP_LOGI(LOG_TAG, "Set SHOW = %i", show);
    gpio_set_level(show_gpio, show ? 1 : 0);
}

void MorioDenkiPlasmaDriver::set_bright(bool bright) {
    ESP_LOGI(LOG_TAG, "Set BRIGHT = %i", bright);
    gpio_set_level(bright_gpio, bright ? 1 : 0);
}

void MorioDenkiPlasmaDriver::write_stride(uint8_t stride) {
    set_databus(stride);
    pulse_clock();
}

void MorioDenkiPlasmaDriver::write_column(uint16_t column) {
    write_stride(column & 0xFF);
    write_stride((column >> 8) & 0xFF);
}

void MorioDenkiPlasmaDriver::write_fanta(const uint8_t * strides, size_t count) {
    // Native pixel format, so just write as is
    for(int i = 0; i < count; i++) {
        write_stride(strides[i]);
    }
}
#endif