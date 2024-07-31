#include <sound/beeper.h>
#include <driver/i2s.h>
#include <algorithm>
#include <sound/waveout.h>

static char LOG_TAG[] = "BOOP";

Beeper::Beeper(): phase{0},
        wavelength{0},
        duration_samples{0},
        active { false },
        bit_state { true },
        channel_status { 0xFF },
        active_channel { -1 } {}

void Beeper::set_frequency(int freq) {
    if(freq > WaveOut::NYQUIST) {
        ESP_LOGE(LOG_TAG, "Cannot %i hz!", freq);
        return;
    }

    wavelength = (WaveOut::BAUD_RATE / freq);
    phase = 0;
    bit_state = true;
    samples = 0;
    ESP_LOGV(LOG_TAG, "Freq=%i, Wavelength=%i", freq, wavelength);
}

void Beeper::set_active(bool a) {
    active = a;
    phase = 0;
    bit_state = true;
    samples = 0;
    ESP_LOGV(LOG_TAG, "Active = %i", a);
}

void Beeper::set_duration(int ms) {
    duration_samples = (WaveOut::BAUD_RATE / 1000) * ms;
    samples = 0;
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

size_t Beeper::fill_buffer(void* buffer, size_t length_) {
    if(!active || wavelength == 0) return 0;

    size_t length = length_;
    if(duration_samples > 0) {
        length = std::min(length, (size_t) (duration_samples / 8) + 1);
    }

    int bits_high = wavelength / 2;
    int bits_low = wavelength / 2;

    uint32_t bit_number = phase;
    uint8_t* buff = (uint8_t*) buffer;
    for(int i = 0; i < length; i++) {
        if((bit_state ? bits_high : bits_low) - bit_number >= 8 && (duration_samples - samples) >= 8) {
            if(bit_state)
                buff[i] = 0xFF;
            else
                buff[i] = 0x00;
            bit_number += 8;
            samples += 8;
            if(bit_number >= (bit_state ? bits_high : bits_low)) {
                bit_number -= (bit_state ? bits_high : bits_low);
                bit_state = !bit_state;
            }
        } else {
            buff[i] = 0x0;
            for(int j = 0; j < 8; j++) {
                bit_number++;
                samples++;
                if(duration_samples == 0 || duration_samples > samples) {
                    if(bit_state) {
                        buff[i] |= (1 << (7 - j));
                    }
                    if(bit_number >= (bit_state ? bits_high : bits_low)) {
                        bit_number -= (bit_state ? bits_high : bits_low);
                        bit_state = !bit_state;
                    }
                }
            }
        }
        if(duration_samples != 0 && samples >= duration_samples) {
            active = false;
            phase = 0;
            bit_state = true;
            return length;
        }
    }
    phase = bit_number;
    return length;
}

void Beeper::start_tone(beeper_channel_t ch, uint freq, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;

    set_duration(0);
    set_frequency(freq);
    set_active(true);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    active_channel = -1;

    set_active(false);
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    
    set_duration(len);
    set_frequency(freq);
    set_active(true);
    vTaskDelay(len);

    active_channel = -1;
}