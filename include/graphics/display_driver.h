#pragma once
#include <device_config.h>

class DisplayDriver {
public:
    virtual void initialize();
    /// @brief Reset the display controller
    virtual void reset();
    /// @brief Send an array of half-columns to the display controller
    virtual void write_fanta(const uint8_t * strides, size_t count);

    /// @brief Enable or disable the display power
    virtual void set_power(bool on);
    /// @brief Show or hide the display contents, while keeping the scanning active
    virtual void set_show(bool show);

#if HAS(VARYING_BRIGHTNESS)
    /// @brief Select between half or full brightness
    virtual void set_bright(bool bright);
#endif
};