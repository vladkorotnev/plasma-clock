#pragma once
#include <driver/i2s.h>
#include <esp32-hal-gpio.h>
#include <functional>

typedef std::function<size_t (uint8_t* buffer, size_t size)> WaveGeneratorCallback;

static const WaveGeneratorCallback NULL_GENERATOR = [](uint8_t*, size_t) { return 0; };

class WaveOut {
public:
    static const int BAUD_RATE = 352800; // <- don't ask why this is 4 times larger, I don't know myself
    static const int NYQUIST = BAUD_RATE / 2; // <- we are not using this to connect a DAC but directly a speaker to the data bus, so the max possible frequency is achieved when outputting 01010101...
    static const size_t RENDER_CHUNK_SIZE = 128;
    
    static void init_I2S(gpio_num_t pin);
    static void set_output_callback(WaveGeneratorCallback callback);

private:
    static const size_t SAMPLE_RATE = 11025;
    static const i2s_port_t I2S_NUM = I2S_NUM_0;
    static const size_t DMA_BUF_LEN = RENDER_CHUNK_SIZE;
    static const size_t DMA_NUM_BUF = 2;
    static bool i2sInited;
    static TaskHandle_t hTask;
    static void task(void*);
    static WaveGeneratorCallback callback;
};

class WaveGenerator {
public:
    virtual size_t fill_buffer(void* buffer, size_t length) { return 0; }
    WaveGeneratorCallback get_callback() {
        return [this](void* buf, size_t len) { return this->fill_buffer(buf, len); };
    }
};
