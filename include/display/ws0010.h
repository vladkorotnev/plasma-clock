#pragma once
#include <graphics/display_driver.h>
#include <device_config.h>

#if HAS(OUTPUT_WS0010)
// Winstar WS0010 compatible OLED display
class Ws0010OledDriver: public DisplayDriver {
public:
    /// @brief Initialize the interface. Configures the GPIO and prepares the outputs for use, also disables the output and high voltage supply.
    /// @note Connect the RWB pin to be LOW as we won't be reading from the display.
    /// @param databus 8 pins connected to the display controller's data bus, DB0 to DB7
    /// @param rs Pin connected to the display controller's RS pin
    /// @param en Pin connected to the display controller's E pin
    Ws0010OledDriver(
        const gpio_num_t databus[8],
        const gpio_num_t rs,
        const gpio_num_t en
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
    gpio_num_t rs_gpio;
    gpio_num_t en_gpio;
    bool is_writing_ddram;
    bool show_state;
    bool is_dim = false;
    uint8_t ddram_ptr;

    inline void set_databus(uint8_t data);
    inline void set_is_command(bool);
    inline void pulse_clock();
    void write_string(const char *);
    void write_stride(uint8_t stride);
};
#endif