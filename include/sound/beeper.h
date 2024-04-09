#pragma once
#include <esp32-hal-gpio.h>

typedef enum beeper_channels {
    CHANNEL_AMBIANCE = 0,
    CHANNEL_NOTICE = 1,

    CHANNEL_ALARM = 6,
    CHANNEL_SYSTEM = 7
} beeper_channel_t;

class Beeper {
public:
    Beeper(gpio_num_t pin, uint8_t ledcChannel);
    void set_channel_state(beeper_channel_t, bool);
    bool is_channel_enabled(beeper_channel_t);

    void start_tone(beeper_channel_t, uint);
    void stop_tone(beeper_channel_t);

    void beep_blocking(beeper_channel_t, uint freq, uint len);
private:
    uint8_t channel_status;
    int active_channel;
    uint8_t pwm_channel;
};