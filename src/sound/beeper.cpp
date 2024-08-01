#include <sound/beeper.h>
#include <driver/i2s.h>
#include <algorithm>
#include <sound/waveout.h>

static char LOG_TAG[] = "BOOP";

Beeper::Beeper(): 
        duration_samples{0},
        samples {0},
        channel_status { 0xFF },
        active_channel { -1 } {
    voice = new SquareGenerator();
}

Beeper::~Beeper() {
    delete voice;
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
    size_t generated = voice->generate_samples(buffer, length, want_samples);
    samples += generated*8;
    if(duration_samples > 0 && duration_samples <= samples) {
        voice->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
    }
    return generated;
}

void Beeper::start_tone(beeper_channel_t ch, uint freq, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;

    duration_samples = 0;
    samples = 0;
    voice->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, freq);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    active_channel = -1;

    voice->set_parameter(ToneGenerator::Parameter::PARAMETER_ACTIVE, false);
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    
    samples = 0;
    duration_samples = len * (WaveOut::BAUD_RATE / 1000);
    voice->set_parameter(ToneGenerator::Parameter::PARAMETER_FREQUENCY, freq);
    vTaskDelay(len);
    active_channel = -1;
}