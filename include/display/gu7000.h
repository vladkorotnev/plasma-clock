#pragma once
#include <graphics/display_driver.h>
#include <device_config.h>

#if HAS(OUTPUT_GU7000)
// Noritake Itron GU7000 series in parallel write-only mode
// For connection diagram, see AppNote APF200 R2.0 paragraph 4.5.5
class ItronGU7000Driver: public DisplayDriver {
public:
    /// @brief Initialize the interface. Configures the GPIO and prepares the outputs for use, also disables the output and high voltage supply.
    /// @note Connect the ~RD pin to Vcc as we won't be reading from the display.
    /// @param databus 8 pins connected to the display controller's data bus, DB0 to DB7
    /// @param wr Pin connected to the display controller's ~WR pin
    /// @param busy Pin connected to the display controller's BUSY pin
    ItronGU7000Driver(
        const gpio_num_t databus[8],
        const gpio_num_t wr,
        const gpio_num_t busy
    );

    void initialize();
    /// @brief Reset the display controller
    void reset();
    void clear();

    /// @brief Enable or disable the high voltage supply 
    void set_power(bool on);
    /// @brief Show or hide the display contents, while keeping the scanning active
    void set_show(bool show);

    /// @brief Enable or disable virtual dimming (using BFI, might flicker depending on FPS stability)
    void set_bright(bool bright);

    /// @brief Send an array of half-columns to the display controller
    void write_fanta(const uint8_t * strides, size_t count);

private:
    gpio_num_t databus_gpios[8];
    gpio_num_t wr_gpio;
    gpio_num_t busy_gpio;
    bool show_state;

    inline void set_databus(uint8_t data);
    inline void pulse_clock();
    void write_string(const char *);
    inline void wait_shit_through();
};
#endif