#include <sound/beeper.h>
#include <esp32-hal-ledc.h>
#include <driver/ledc.h>

static void ledcWriteToneD(uint8_t chan, uint32_t freq, uint32_t duty)
{
    if(!freq){
        ledcWrite(chan, 0);
        return;
    }

    uint8_t group=(chan/8), timer=((chan/2)%4);

    ledc_timer_config_t ledc_timer = {
        .speed_mode       = (ledc_mode_t) group,
        .duty_resolution  = (ledc_timer_bit_t) 10,
        .timer_num        = (ledc_timer_t) timer,
        .freq_hz          = freq, 
        .clk_cfg          = (ledc_clk_cfg_t) 0
    };

    if(ledc_timer_config(&ledc_timer) != ESP_OK)
    {
        log_e("ledcSetup failed!");
        return;
    }

    ledcWrite(chan, duty);
}

static portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

Beeper::Beeper(gpio_num_t pin, uint8_t ch) {
    pwm_channel = ch;
    channel_status = 0xFF;
    active_channel = -1;

    ledcSetup(pwm_channel, 10000, 8);
    ledcAttachPin(pin, pwm_channel);
    ledcWrite(pwm_channel, 0);
}

void Beeper::set_channel_state(beeper_channel_t ch, bool active) {
    if(active) {
        channel_status |= (1 << ch);
    } else {
        if(active_channel == ch) {
            stop_tone(ch);
        }
        channel_status &= ~(1 << ch);
    }
}

bool Beeper::is_channel_enabled(beeper_channel_t ch) {
    return (channel_status & (1 << ch)) > 0;
}

void Beeper::start_tone(beeper_channel_t ch, uint freq, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    ledcWriteToneD(pwm_channel, freq, duty);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    ledcWrite(pwm_channel, 0);
    active_channel = -1;
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len, uint16_t duty) {
    taskENTER_CRITICAL(&_spinlock);
    start_tone(ch, freq, duty);
    delayMicroseconds(len * 1000);
    stop_tone(ch);
    taskEXIT_CRITICAL(&_spinlock);
}