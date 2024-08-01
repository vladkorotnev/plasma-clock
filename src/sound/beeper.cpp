#include <sound/beeper.h>
#include <driver/i2s.h>
#include <algorithm>
#include <sound/waveout.h>

static char LOG_TAG[] = "BOOP";

class SquareGenerator: public ToneGenerator {
public:
    SquareGenerator():
        phase{0},
        wavelength{0},
        active { false },
        duty_offset { 0 }
    {}

    void set_frequency(int freq) {
        if(freq > WaveOut::NYQUIST) {
            ESP_LOGE(LOG_TAG, "Cannot %i hz!", freq);
            return;
        }

        wavelength = (WaveOut::BAUD_RATE / freq);
        phase = 0;
        ESP_LOGV(LOG_TAG, "Freq=%i, Wavelength=%i", freq, wavelength);
    }

    void set_active(bool a) {
        active = a;
        phase = 0;
        ESP_LOGV(LOG_TAG, "Active = %i", a);
    }

    void set_parameter(Parameter p, int v) override {
        switch(p) {
            case PARAMETER_ACTIVE:
                set_active(v != 0);
                break;

            case PARAMETER_FREQUENCY:
                set_frequency(v);
                break;

            default: break;
        }
    }

    size_t generate_samples(void* buffer, size_t length, uint32_t want_samples_) override {
        if(!active || wavelength == 0) return 0;

        int bits_high = wavelength / 2 - duty_offset;
        uint8_t* buff = (uint8_t*) buffer;
        uint32_t want_samples = want_samples == 0 ? (length * 8) : std::min(want_samples_, length * 8);
        size_t idx = 0;
        int bit = 7;

        for(int s = 0; s < want_samples; s++) {
            bool state = (phase < bits_high);
            idx = s / 8;
            bit = 8 - (s % 8);
            if(state) {
                buff[idx] |= (1 << bit);
            }
            phase = (phase + 1) % wavelength;
        }
        
        return idx + 1;
    }

private:
    int phase;
    int wavelength;
    int duty_offset;
    bool active;
};

Beeper::Beeper(): 
        duration_samples{0},
        samples {0},
        channel_status { 0xFF },
        active_channel { -1 } {
    for(int i = 0; i < POLY_VOICES; i++) {
        voices[i] = new SquareGenerator();
    }
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
    int want_samples = duration_samples == 0? 0: (duration_samples - samples);
    size_t length = duration_samples == 0? length_ : std::min(length_, (size_t) (want_samples / 8));
    size_t generated = 0;
    for(int i = 0; i < POLY_VOICES; i++) {
        size_t cur = voices[i]->generate_samples(buffer, length, want_samples); //<- voices are expected to mix on their own
        if(cur > generated) generated = cur;
    } 
    samples += generated*8;
    if(duration_samples > 0 && duration_samples <= samples) {
        // quiescent time has ended, in here you would take the next sequence step and enact it
        // then update the duration to the next one.
        // for now we just beep... or unbeep
        voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
    }
    return generated;
}

void Beeper::start_tone(beeper_channel_t ch, uint freq, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;

    duration_samples = 0;
    samples = 0;
    voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, freq);
    voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, true);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    active_channel = -1;

    voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    
    samples = 0;
    duration_samples = len * (WaveOut::BAUD_RATE / 1000);
    voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, freq);
    voices[DEFAULT_BEEP_VOICE_IDX]->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, true);
    vTaskDelay(len);
    active_channel = -1;
}