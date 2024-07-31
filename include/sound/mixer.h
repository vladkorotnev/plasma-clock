#pragma once
#include <stddef.h>
#include <sound/waveout.h>

class WaveGenerator {
public:
    virtual size_t fill_buffer(void* buffer, size_t length) { return 0; }
    WaveGeneratorCallback get_callback() {
        return [this](uint8_t* buf, size_t len) { return this->fill_buffer(buf, len); };
    }
};

// class WaveMixer: public WaveGenerator {
// public:
//     WaveMixer() {}
//     size_t fill_buffer(void* buffer, size_t length) override {
//         size_t written = 0;
//         uint8_t tmp[length] = { 0 };
//         uint8_t* buf = (uint8_t*) buffer;
//         for(mixer_channel_t ch = (mixer_channel_t) 0; ch < CHANNEL_MAX_INVALID; ch = (mixer_channel_t) (ch + 1)) {
//             if(is_channel_on(ch)) {
//                 size_t cur_ch_samples = generators[ch](tmp, length);
//                 if(cur_ch_samples > written) written = cur_ch_samples;
//                 for(int i = 0; i < cur_ch_samples; i++) {
//                     buf[i] |= tmp[i];
//                 }
//             }
//         }
//         return written;
//     }

//     void set_channel_state(mixer_channel_t channel, bool sts) {
//         if(sts) {
//             channel_status |= (1 << channel);
//         } else {
//             channel_status &= ~(1 << channel);
//         }
//     }

//     void set_channel_callback(mixer_channel_t channel, const WaveGeneratorCallback cb) {
//         generators[channel] = cb;
//     }

// private:
//     WaveGeneratorCallback generators[CHANNEL_MAX_INVALID] = { NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR, NULL_GENERATOR };
//     uint8_t channel_status = 0xFF;
//     bool is_channel_on(mixer_channel_t ch) { return (channel_status & (1 << ch)) != 0; }
// };