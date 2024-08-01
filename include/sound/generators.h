#pragma once
#include <sound/waveout.h>

class ToneGenerator {
public:
    enum Parameter {
        PARAMETER_FREQUENCY,
        PARAMETER_ACTIVE
    };
    virtual size_t generate_samples(void* buffer, size_t length, uint32_t wanted_samples) { return 0; }
    virtual void set_parameter(Parameter p, int v) { }
};


class SquareGenerator: public ToneGenerator {
public:
    SquareGenerator():
        phase{0},
        wavelength{0},
        active { false },
        duty_offset { 0 }
    {}

    void set_frequency(int freq) {
        if(freq == 0) return;
        if(freq > WaveOut::NYQUIST) {
            ESP_LOGE("SQGEN", "Cannot %i hz!", freq);
            return;
        }

        wavelength = (WaveOut::BAUD_RATE / freq);
        phase = 0;
    }

    void set_active(bool a) {
        active = a;
        phase = 0;
    }

    void set_parameter(Parameter p, int v) override {
        switch(p) {
            case PARAMETER_ACTIVE:
                set_active(v != 0);
                break;

            case PARAMETER_FREQUENCY:
                set_frequency(v);
                set_active(v != 0);
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
