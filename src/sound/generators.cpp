#include <sound/generators.h>
#include <algorithm>

SquareGenerator::SquareGenerator():
        phase{0},
        wavelength{0},
        active { false },
        duty { 2 }
    {}

void SquareGenerator::set_frequency(int freq) {
    if(freq == 0) return;
    if(freq > WaveOut::NYQUIST) {
        ESP_LOGE("SQGEN", "Cannot %i hz!", freq);
        return;
    }

    wavelength = (WaveOut::BAUD_RATE / freq);
    phase = 0;
}

void SquareGenerator::set_active(bool a) {
    active = a;
    phase = 0;
}

void SquareGenerator::reset_phase() {
    phase = 0;
}

void SquareGenerator::set_parameter(Parameter p, int v) {
    switch(p) {
        case PARAMETER_ACTIVE:
            set_active(v != 0);
            break;

        case PARAMETER_FREQUENCY:
            set_frequency(v);
            set_active(v != 0);
            break;

        case PARAMETER_DUTY:
            if(abs(v) < 2) duty = 2;
            else duty = v;
            break;

        default: break;
    }
}

size_t SquareGenerator::generate_samples(void* buffer, size_t length, uint32_t want_samples_) {
    if(!active || wavelength == 0) return 0;

    int bits_high = wavelength / abs(duty);
    if(duty < 0) bits_high = wavelength - bits_high;
    uint8_t* buff = (uint8_t*) buffer;
    uint32_t want_samples = want_samples_ == 0 ? (length * 8) : std::min(want_samples_, length * 8);
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


static const int NOISETAB_SIZE = 256;
static uint8_t NOISETAB[NOISETAB_SIZE] = { 0 };
static bool NOISETAB_READY = false;

static void _create_noisetab_if_needed() {
    if(!NOISETAB_READY) {
        esp_fill_random(NOISETAB, NOISETAB_SIZE);
        NOISETAB_READY = true;
    }
}

NoiseGenerator::NoiseGenerator():
    phase { 0 },
    wavelength { 0 },
    active { false } {
        rng = 1;
}

void NoiseGenerator::set_parameter(Parameter p, int v) {
    switch(p) {
        case PARAMETER_ACTIVE:
            active = (v != 0);
            phase = 0;
            break;

        case PARAMETER_FREQUENCY:
            if(v == 0) {
                active = false;
            }
            else {
                wavelength = (WaveOut::BAUD_RATE / (v * 2));
                active = true;
            }
            break;

        case PARAMETER_DUTY:
            // Not supported
            break;

        default: break;
    }
}

size_t NoiseGenerator::generate_samples(void* buffer, size_t length, uint32_t want_samples_) {
    if(!active || wavelength == 0) return 0;

    uint8_t* buff = (uint8_t*) buffer;
    uint32_t want_samples = want_samples_ == 0 ? (length * 8) : std::min(want_samples_, length * 8);
    size_t idx = 0;
    int bit = 7;

    for(int s = 0; s < want_samples; s++) {
        idx = s / 8;
        bit = 8 - (s % 8);
        if(state && (rng & 1) > 0) {
            buff[idx] |= (1 << bit);
        }
        phase = (phase + 1) % wavelength;
        if(phase == 0) {
            state ^= 1;
            if (state) {
                // random number generator from MAME:
                // https://github.com/mamedev/mame/blob/master/src/devices/sound/ay8910.cpp

                rng ^= (((rng & 1) ^ ((rng >> 3) & 1)) << 17);
                rng >>= 1;
            }
        }
    }
    
    return idx + 1;
}