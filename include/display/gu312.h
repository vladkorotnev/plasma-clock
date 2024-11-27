#pragma once
#include <graphics/display_driver.h>
#include <device_config.h>

#if HAS(OUTPUT_GU312)
class NoritakeGu312Driver: public DisplayDriver {
public:
    /// @brief Initialize the interface. Configures the GPIO and prepares the outputs for use, also disables the output and high voltage supply.
    /// @note Connect the CS to LOW
    /// @param databus 8 pins connected to the display controller's data bus, DB0 to DB7
    /// @param cd Pin connected to the display controller's C/D pin
    /// @param wr Pin connected to the display controller's WR pin
    /// @param blank Pin connected to the display controller's blanking pin
    /// @param vsync Pin connected to the display controller's FEP pin (optional)
    NoritakeGu312Driver(
        const gpio_num_t databus[8],
        const gpio_num_t cd,
        const gpio_num_t wr,
        const gpio_num_t blank,
        const gpio_num_t vsync = gpio_num_t::GPIO_NUM_NC
    );

    void initialize();
    /// @brief Reset the display controller
    void reset();
    void clear();

    /// @brief Enable or disable the high voltage supply 
    void set_power(bool on);

    /// @brief Enable or disable dimming
    void set_bright(bool bright);

    /// @brief Send an array of half-columns to the display controller
    void write_fanta(const uint8_t * strides, size_t count);

private:
    gpio_num_t databus_gpios[8];
    gpio_num_t cd_gpio;
    gpio_num_t wr_gpio;
    gpio_num_t blank_gpio;
    gpio_num_t fep_gpio;

    void send_command(uint8_t cmd);
    inline void set_databus(uint8_t data);
    inline void set_is_command(bool);
    inline void pulse_clock();
};
#endif