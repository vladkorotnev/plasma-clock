#include <sound/beeper.h>
#include <driver/i2s.h>
#include <algorithm>

static char LOG_TAG[] = "BOOP";


TaskHandle_t hTask = NULL;
static bool i2sInited = false;
#define I2S_NUM I2S_NUM_0
#define DMA_BUF_LEN     (128)
#define DMA_NUM_BUF     (2)
#define SAMPLE_RATE 11025
static const int max_size {DMA_BUF_LEN};
static uint32_t baud {352800};
static Synthesizerish *synth = nullptr;

const int nyquist = baud / 2;

static void beeper_task(void*) {
    uint8_t chunk[max_size];
    uint8_t null[max_size] = { 0xFF };
    size_t out_size = 0;
    while(1) {
        if(synth != nullptr) {
            size_t write_max = synth->fill_buffer(chunk, max_size);
            if(write_max > 0) {
                for(size_t i = 0; i < write_max; i++) chunk[i] = ~chunk[i];
                i2s_write(I2S_NUM, chunk, write_max, &out_size, portMAX_DELAY);
            } else {
                i2s_write(I2S_NUM, null, max_size, &out_size, portMAX_DELAY);
            }
            taskYIELD();
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void Beeper::InitI2S(gpio_num_t pin) {
    if(i2sInited) return;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = DMA_NUM_BUF,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = false,
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
    };

    i2s_pin_config_t pincfg = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = I2S_PIN_NO_CHANGE,
        .data_out_num = pin,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

    i2s_set_pin(I2S_NUM, &pincfg);
   
    xTaskCreate(
        beeper_task,
        "BOOP",
        4096,
        nullptr,
        configMAX_PRIORITIES - 1,
        &hTask
    );

    i2sInited = true;
}


class Beeper::Buzzer: public Synthesizerish {
public:
    Buzzer():
        phase{0},
        wavelength{0},
        duration_samples{0},
        active { false },
        bit_state { true }
    {}

    void set_frequency(int freq) {
        if(freq > nyquist) {
            ESP_LOGE(LOG_TAG, "Cannot %i hz!", freq);
            return;
        }

        wavelength = (baud / freq);
        phase = 0;
        bit_state = true;
        samples = 0;
        ESP_LOGV(LOG_TAG, "Freq=%i, Wavelength=%i", freq, wavelength);
    }

    void set_active(bool a) {
        active = a;
        phase = 0;
        bit_state = true;
        samples = 0;
        ESP_LOGV(LOG_TAG, "Active = %i", a);
    }

    void set_duration(int ms) {
        duration_samples = (baud / 1000) * ms;
        samples = 0;
    }

    size_t fill_buffer(void* buffer, size_t length_) {
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

private:
    int samples;
    int phase;
    int wavelength;
    int duration_samples;
    bool active;
    bool bit_state;
};

Beeper::Beeper() {
    channel_status = 0xFF;
    active_channel = -1;
    fallback_synthesizer = new Buzzer();
    synth = fallback_synthesizer;
}

Beeper::~Beeper() {
    delete fallback_synthesizer;
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

void Beeper::start_tone(beeper_channel_t ch, uint freq, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;
    
    fallback_synthesizer->set_duration(0);
    fallback_synthesizer->set_frequency(freq);
    fallback_synthesizer->set_active(true);
}

void Beeper::stop_tone(beeper_channel_t ch) {
    if(active_channel != ch) return;
    active_channel = -1;

    fallback_synthesizer->set_active(false);
}

void Beeper::beep_blocking(beeper_channel_t ch, uint freq, uint len, uint16_t duty) {
    if(active_channel > ch) return;
    if(!is_channel_enabled(ch)) return;

    if(active_channel < ch) active_channel = ch;

    fallback_synthesizer->set_duration(len);
    fallback_synthesizer->set_frequency(freq);
    fallback_synthesizer->set_active(true);
    vTaskDelay(len);

    active_channel = -1;
}