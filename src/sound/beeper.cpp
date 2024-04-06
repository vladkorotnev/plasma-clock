#include <sound/beeper.h>

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

void Beeper::start_tone(beeper_channel_t ch, uint freq) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    ledcWriteTone(pwm_channel, freq);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    ledcWrite(pwm_channel, 0);
    active_channel = -1;
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len) {
    start_tone(ch, freq);
    vTaskDelay(pdMS_TO_TICKS(len));
    stop_tone(ch);
}