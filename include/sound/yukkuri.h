#pragma once
#include <device_config.h>
#include <sound/waveout.h>
#include <queue>
#include <optional>
#if HAS(AQUESTALK)
#include <aquestalk.h>
#else
#define AQ_SIZE_WORKBUF 0 // avoid using extra RAM
#endif

typedef std::function<void (bool completed)> YukkuriUtteranceCallback;
struct YukkuriUtterance {
    const char * text = "";
    uint8_t speed = 0xFF;
    uint16_t pause = 0xFFFF;
    YukkuriUtteranceCallback callback = [](bool) {};

    YukkuriUtterance(const char * txt, YukkuriUtteranceCallback cb = [](bool){}, uint8_t sp = 0xFF, uint16_t pa = 0xFFFFu) {
        text = txt;
        speed = sp;
        pause = pa;
        callback = cb;
    }
};

class Yukkuri: public WaveGenerator {
public:
    Yukkuri(const char * license_key, uint16_t frame_length = 32);

    bool speak(YukkuriUtterance& utterance);
    bool speak(const char * text, YukkuriUtteranceCallback callback = [](bool){});
    void cancel_current();
    void cancel_all();
    bool is_speaking();

    size_t fill_buffer(void* buffer, size_t length) override;
private:
    static const uint8_t QUEUE_MAX_LENGTH = 4;
    static const int STRETCH_FACTOR = WaveOut::BAUD_RATE / 8000; //<- 8kHz from AquesTalk Pico

    bool ready = false;
    
    bool out_state = false;
    int out_phase = 0;
    static const int16_t HYST_ZERO_MARGIN = 1024;
    static const int16_t HYST_ONE_MARGIN = 1800;

    bool speaking = false;
    uint32_t workbuf[AQ_SIZE_WORKBUF];

    uint16_t pcm_buf_size = 0;
    uint16_t pcm_buf_length = 0;
    uint16_t pcm_playhead = 0;
    int16_t * pcm_buf = nullptr;

    std::queue<YukkuriUtterance> queue = {};
    void _finish_current();
    void _start_next_utterance_if_needed();
    void _stop_speech();
};