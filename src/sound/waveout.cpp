#include <sound/waveout.h>
#include <os_config.h>

static char LOG_TAG[] = "WOUT";

bool WaveOut::i2sInited = false;
TaskHandle_t WaveOut::hTask = NULL;
WaveGeneratorCallback WaveOut::callback[] = {NULL_GENERATOR, NULL_GENERATOR};

void WaveOut::init_I2S(gpio_num_t pin) {
    if(WaveOut::i2sInited) return;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_8BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = DMA_NUM_BUF,
        .dma_buf_len = DMA_BUF_LEN,
        .use_apll = true,
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
        task,
        "WaveOut",
        4096,
        nullptr,
        pisosTASK_PRIORITY_WAVEOUT,
        &hTask
    );

    WaveOut::i2sInited = true;
}

void WaveOut::task(void*) {
    static uint8_t chunk[RENDER_CHUNK_SIZE+1] = { 0x0 };
    static const uint8_t null[RENDER_CHUNK_SIZE+1] = { 0xFF };
    static uint32_t * fast_chunk = (uint32_t*) chunk;
    const size_t fast_chunk_size = RENDER_CHUNK_SIZE / sizeof(uint32_t);
    static size_t out_size = 0;
    while(1) {
        size_t total = 0;
        for(int i = 0; i < CHANNEL_COUNT; i++) {
            size_t generated_bytes = callback[i](chunk, RENDER_CHUNK_SIZE);
            if(generated_bytes > total) total = generated_bytes;
        }
        if(total > 0) {
            for(size_t i = 0; i < fast_chunk_size; i++) fast_chunk[i] = ~fast_chunk[i];
            i2s_write(I2S_NUM, chunk, total, &out_size, portMAX_DELAY);
            memset(chunk, 0, RENDER_CHUNK_SIZE+1);
        } else {
            i2s_write(I2S_NUM, null, RENDER_CHUNK_SIZE, &out_size, portMAX_DELAY);
        }
        taskYIELD();
    }
}

void WaveOut::set_output_callback(int channel, WaveGeneratorCallback cb) {
    callback[channel] = cb;
}