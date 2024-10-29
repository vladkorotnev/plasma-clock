#pragma once
#include <graphics/display_driver.h>
#include <device_config.h>

#if HAS(OUTPUT_AKIZUKI_K875)
#include <driver/spi_common.h>
#include <driver/spi_master.h>

/// A driver for the Akizuki Denshi K-875 LED panel module.
/// Each panel consists of 3 shift registers with a common clock and latch signal.
/// The register at SIN1 selects which row is lit up, and the ones at SIN2 and SIN3 choose
/// which LEDs in that row are lit up.
///
/// Thus, this driver performs dynamic driving by the means of sweeping each row of the whole set
/// of panels at a high enough speed to not be noticeable by the human eye.
/// NB: EXPERIMENTAL!
class AkizukiK875Driver: public DisplayDriver {
public:
    /// @brief Initialize an instance of the driver for the K-875
    /// @param latch_pin Pin to the LATCH signal
    /// @param clock_pin Pin to the CLOCK signal
    /// @param strobe_pin Pin to the STROBE (SHOW) signal
    /// @param sin1_pin Pin to the SIN1 (vertical sweep data) signal
    /// @param sin2_pin Pin to the SIN2 (LED1 panel data) signal
    /// @param sin3_pin Pin to the SIN3 (LED2 panel data) signal
    /// @param sacrificial_pin Sacrificial pin used in setting up the QIO SPI driver. Not used, can be freed after the driver was set up.
    /// @param ledc_channel LEDC channel for brightness control. Default is 1.
    /// @param bright_pwm PWM duty cycle in bright state (255 = display off, 0 = full brightness)
    /// @param dark_pwm PWM duty cycle in dark state (255 = display off, 0 = full brightness)
    /// @param panel_count Count of daisy chained boards in the row. Default is 4 boards (8 panels, 128px horizontal).
    /// @param desired_frame_clock Desired refresh rate for the whole screen in Hz. Default is 120. Too high can congest the CPU with interrupts, too low causes flickering.
    /// @param host SPI host to use
    AkizukiK875Driver(
        gpio_num_t latch_pin,
        gpio_num_t clock_pin,
        gpio_num_t strobe_pin,
        gpio_num_t sin1_pin,
        gpio_num_t sin2_pin,
        gpio_num_t sin3_pin,
        gpio_num_t sacrificial_pin,
        uint8_t ledc_channel = 1,
        int bright_pwm = 8,
        int dark_pwm = 200,
        uint8_t panel_count = 4,
        uint8_t desired_frame_clock = 80,
        spi_host_device_t host = SPI2_HOST
    );

    void initialize() override;

    /// @brief Enable or disable the display power
    void set_power(bool on) override;
    /// @brief Show or hide the display contents, while keeping the scanning active
    void set_show(bool show) override;
    /// @brief Select between half or full brightness
    void set_bright(bool bright) override;
    /// @brief Send an array of half-columns to the display controller
    void write_fanta(const uint8_t * strides, size_t count) override;
    
protected:
    const int PIXEL_CLOCK_HZ;
    const uint8_t PANEL_COUNT;
    const gpio_num_t LATCH_PIN;
    const gpio_num_t SIN1_PIN;
    const gpio_num_t SIN2_PIN;
    const gpio_num_t SIN3_PIN;
    const gpio_num_t CLOCK_PIN;
    const gpio_num_t STROBE_PIN;
    const gpio_num_t SACRIFICIAL_UNUSE_PIN;
    uint8_t * data;
    const uint8_t ledcChannel;
    const int brightPwm;
    const int darkPwm;
    int curPwm;
    int targetPwm;
    const spi_host_device_t spi;
    const size_t total_bytes_per_row;
    bool power_state = true;
    bool show_state = true;
    bool bright_state = true;
    void update_strobe_value();
};


#endif