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

int SquareGenerator::get_parameter(Parameter p) {
    switch(p) {
        case PARAMETER_ACTIVE:
            return active;
        case PARAMETER_FREQUENCY:
            return (wavelength == 0) ? 0 : (WaveOut::BAUD_RATE / wavelength);
        case PARAMETER_DUTY:
            return duty;
        default:
            return 0;
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

int NoiseGenerator::get_parameter(Parameter p) {
    switch(p) {
        case PARAMETER_ACTIVE:
            return active;
        case PARAMETER_FREQUENCY:
            return (wavelength == 0) ? 0 : (WaveOut::BAUD_RATE / wavelength / 2);
        default:
            return 0;
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

Sampler::Sampler():
    playhead { 0 },
    waveform { nullptr },
    remaining_samples { 0 },
    stretch_factor { 1 },
    active { false },
    state { false }
{}

void Sampler::load_sample(const rle_sample_t * s) {
    waveform = s;
    rewind();
    stretch_factor = 1;
}

void Sampler::rewind() {
    if(waveform == nullptr || waveform->rle_data == nullptr || waveform->length == 0) return;
    playhead = 0;
    remaining_samples = waveform->rle_data[0];
    while(remaining_samples == 0 && playhead < waveform->length) {
        playhead++;
        remaining_samples = waveform->rle_data[playhead];
        state ^= 1;
    }
}

void Sampler::set_parameter(Parameter p, int v) {
    switch(p) {
        case PARAMETER_ACTIVE:
            active = (v != 0);
            rewind();
            break;

        case PARAMETER_FREQUENCY:
            if(waveform == nullptr) return;
            if(v == 0) {
                active = false;
            }
            else {
                frequency = v;
                // since the sample rate of the snippet is always lower than that of WaveOut
                // we can only care about the stretch factor
                stretch_factor = (WaveOut::BAUD_RATE / waveform->sample_rate) * waveform->root_frequency / v;
                ESP_LOGV("SAMP", "New stretch = %i, skip = %i", stretch_factor, skip_factor);
                active = true;
                rewind();
            }
            break;

        case PARAMETER_SAMPLE_ADDR:
            if(v == 0) return;
            load_sample((const rle_sample_t*) v);
            break;

        default: break;
    }
}

int Sampler::get_parameter(Parameter p) {
    switch(p) {
        case PARAMETER_ACTIVE:
            return active;
        case PARAMETER_FREQUENCY:
            return frequency;
        case PARAMETER_SAMPLE_ADDR:
            return (int) waveform;
        default:
            return 0;
    }
}

size_t Sampler::generate_samples(void * buffer, size_t length, uint32_t want_samples_) {
    if(!active || waveform == nullptr || waveform->length == 0 || waveform->rle_data == nullptr)
        return 0;

    uint8_t* buff = (uint8_t*) buffer;
    uint32_t want_samples = want_samples_ == 0 ? (length * 8) : std::min(want_samples_, length * 8);
    size_t idx = 0;
    int bit = 7;

    for(int s = 0; s < want_samples; s++) {
        idx = s / 8;
        bit = 8 - (s % 8);
        
        if(state) {
            buff[idx] |= (1 << bit);
        }

        if(stretch_factor == 1 || (s > 0 && (s % stretch_factor) == 0)) {
            remaining_samples -= 1;

            if(remaining_samples == 0) {
                playhead = (playhead + 1) % waveform->length;
                remaining_samples = waveform->rle_data[playhead];
                state ^= 1;
            }
        }
    }
    
    return idx + 1;
}
