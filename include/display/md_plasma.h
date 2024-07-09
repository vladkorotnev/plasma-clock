#pragma once
#include <device_config.h>

#if HAS(OUTPUT_MD_PLASMA)
#include <hal/gpio_hal.h>
#include <graphics/display_driver.h>

/// @brief Interface to the Morio Denki plasma display controller board
class MorioDenkiPlasmaDriver: public DisplayDriver {
public:
    /// @brief Initialize the interface. Configures the GPIO and prepares the outputs for use, also disables the output and high voltage supply.
    /// @param databus 8 pins connected to the display controller's pixel data bus, LSB to MSB
    /// @param clock Pin connected to the display controller's CLK pin
    /// @param reset Pin connected to the display controller's RST pin
    /// @param bright Pin connected to the display controller's BRIGHT pin
    /// @param blanking Pin connected to the display controller's SHOW pin
    /// @param hv_enable Pin connected to the high voltage supply's ENABLE pin
    MorioDenkiPlasmaDriver(
        const gpio_num_t databus[8],
        const gpio_num_t clock,
        const gpio_num_t reset,
        const gpio_num_t bright,
        const gpio_num_t blanking,
        const gpio_num_t hv_enable
    );
    
    /// @brief Reset the display controller
    void reset();

    /// @brief Enable or disable the high voltage supply 
    void set_power(bool on);
    /// @brief Show or hide the display contents, while keeping the scanning active
    void set_show(bool show);
    /// @brief Select between half or full brightness
    void set_bright(bool bright);

    /// @brief Send an array of half-columns to the display controller
    void write_fanta(const uint8_t * strides, size_t count);

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
    /// @brief Send a half-column to the display controller
    void write_stride(uint8_t stride);
    /// @brief Send a full column to the display controller
    void write_column(uint16_t column);
};

#endif