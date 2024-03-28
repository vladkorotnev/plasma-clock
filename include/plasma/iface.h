#pragma once
#include <hal/gpio_hal.h>

class PlasmaDisplayIface {
public:
    PlasmaDisplayIface(
        const gpio_num_t databus[8],
        const gpio_num_t clock,
        const gpio_num_t reset,
        const gpio_num_t bright,
        const gpio_num_t blanking,
        const gpio_num_t hv_enable
    );
    
    void reset();

    void set_power(bool on);
    void set_show(bool show);
    void set_bright(bool bright);

    void write_stride(uint8_t stride);
    void write_column(uint16_t column);

private:
    gpio_num_t databus_gpios[8];
    gpio_num_t clk_gpio;
    gpio_num_t reset_gpio;
    gpio_num_t bright_gpio;
    gpio_num_t show_gpio;
    gpio_num_t hv_en_gpio;
    void initialize();
    inline void set_databus(uint8_t data);
    inline void pulse_clock();
};