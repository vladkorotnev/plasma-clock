#pragma once
#include <device_config.h>

class DisplayDriver {
public:
    /// @brief Reset the display controller
    virtual void reset();
    /// @brief Send a half-column to the display controller
    virtual void write_stride(uint8_t stride);

    /// @brief Enable or disable the display power
    virtual void set_power(bool on);
    /// @brief Show or hide the display contents, while keeping the scanning active
    virtual void set_show(bool show);

#if HAS(VARYING_BRIGHTNESS)
    /// @brief Select between half or full brightness
    virtual void set_bright(bool bright);
#endif
};