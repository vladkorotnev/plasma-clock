#include <sound/beeper.h>
#include <driver/spi_master.h>
#include <SPI.h>
#include <freertos/ringbuf.h>
#include <vector>

static char LOG_TAG[] = "BOOP";


TaskHandle_t hTask = NULL;
static bool spiInited = false;
/// Interface config of SPI device
static spi_device_interface_config_t if_cfg;
/// Interface config of SPI bus
static spi_bus_config_t bus_cfg;
/// SPI device handle
static spi_device_handle_t handle;

static spi_host_device_t host {SPI3_HOST};
static uint8_t mode {SPI_MODE3};
static int dma_chan {2};
static const int max_size {256};
static uint32_t baud {400000};
#define BUFFERS 8
static Synthesizerish *synth = nullptr;

static uint8_t* dma;
static RingbufHandle_t ringbuf;
static StaticRingbuffer_t _ringbuf { 0 };

const int nyquist = baud / 2;


inline void output_buffer(void* buf, size_t len) {
    // Begin an SPI transaction
    static spi_transaction_t t { 0 };
    t.length = 8 * len;  // set Tx size in bits
    t.tx_buffer = buf; // Tx from DMA buffer
    t.rxlength = 0;

    // Send data over SPI interface
    spi_device_polling_transmit(handle, &t);
}

static void output_task(void*) {
    esp_err_t e = spi_device_acquire_bus(handle, portMAX_DELAY);
    if (e != ESP_OK) {
        ESP_LOGE(LOG_TAG, "[ERROR] SPI device lock failed : %d\n", e);
    }

    uint8_t * chunk = NULL;
    size_t len;
    while(1) {
        chunk = (uint8_t*) xRingbufferReceive(ringbuf, &len, portMAX_DELAY);
        if(chunk != NULL) {
            output_buffer(chunk, len);
            vRingbufferReturnItem(ringbuf, chunk);
        }
    }

    spi_device_release_bus(handle);
}

static void beeper_task(void*) {
    uint8_t chunk[max_size * 2];
    while(1) {
        if(synth != nullptr) {
            size_t write_max = synth->fill_buffer(chunk, max_size * 2);
            if(write_max > 0) {
                xRingbufferSend(ringbuf, chunk, write_max, portMAX_DELAY);
            } else {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


void Beeper::InitSPI(gpio_num_t pin) {
    if(spiInited) return;

    bus_cfg.sclk_io_num = -1;
    bus_cfg.miso_io_num = -1;
    bus_cfg.mosi_io_num = pin;
    if_cfg.spics_io_num = -1; //<- CS pin is selected at interface level

    // Set max transfer size in bytes
    bus_cfg.max_transfer_sz = max_size;
    
    // Suitable configuration found experimentally
    if_cfg.mode = mode;
    if_cfg.clock_speed_hz = baud;
    if_cfg.queue_size = BUFFERS;
    if_cfg.flags = SPI_DEVICE_NO_DUMMY;
    if_cfg.pre_cb = NULL;
    if_cfg.post_cb = NULL;
    if_cfg.dummy_bits = 0;
    if_cfg.command_bits = 0;
    if_cfg.address_bits = 0;
    if_cfg.cs_ena_posttrans = 0;
    if_cfg.cs_ena_pretrans = 0;
    if_cfg.input_delay_ns = 0;
    
    // Open the SPI bus and configure
    esp_err_t e = spi_bus_initialize(host, &bus_cfg, dma_chan);
    if (e != ESP_OK) {
        ESP_LOGE(LOG_TAG, "[ERROR] SPI bus initialize failed : %d\n", e);
    }

    // Set a handle on the SPI bus
    e = spi_bus_add_device(host, &if_cfg, &handle);
    if (e != ESP_OK) {
        ESP_LOGE(LOG_TAG, "[ERROR] SPI bus add device failed : %d\n", e);
    }
    
    dma = (uint8_t*) heap_caps_malloc(BUFFERS * max_size + 1, MALLOC_CAP_DMA);
    ringbuf = xRingbufferCreateStatic(BUFFERS * max_size, RINGBUF_TYPE_BYTEBUF, dma, &_ringbuf);

    xTaskCreate(
        beeper_task,
        "BOOP",
        4096,
        nullptr,
        24,
        &hTask
    );

    xTaskCreate(
        output_task,
        "BEEP",
        4096,
        nullptr,
        23,
        &hTask
    );

    spiInited = true;
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