#include <device_config.h>
#include <SPI.h>
#include <driver/spi_master.h>
static char LOG_TAG[] = "ASS";

/// Interface config of SPI device
spi_device_interface_config_t if_cfg;
/// Interface config of SPI bus
spi_bus_config_t bus_cfg;
/// SPI device handle
spi_device_handle_t handle;

/// Select SPI driver host (either HSPI or VSPI should work)
spi_host_device_t host {VSPI_HOST};
/// Select SPI driver mode (we only use MOSI pin so don't really care)
uint8_t mode {SPI_MODE3};
/// Select DMA channel of SPI, must be 1 or 2
int dma_chan {1};
/// Select max DMA buffer size in BYTES
int max_size {8192};
/// Select SPI baud rate
uint32_t baud {400000};
/// DMA buffer
void* dma_buffer;

const int nyquist = baud / 2;

static void freq_to_buff(uint16_t freq, int soffs = 0, int len = max_size) {
    if(freq > nyquist) {
        ESP_LOGE(LOG_TAG, "Cannot %i hz!", freq);
        return;
    }

    // 2khz = 1010101010 (change bit every bit)
    // 1khz = 1100110011 (change every 2 bits)
    // 500 hz = 111000111000 (every 3 bits)
    // adding 1 bit halves the frequency

   int bits_per_half_wavelength = (baud / freq) / 2;
   ESP_LOGI(LOG_TAG, "bits per half wave = %i", bits_per_half_wavelength);

   uint32_t bit_number = 0;
   uint8_t * buff = (uint8_t*) dma_buffer;
   bool bit_state = true;
   for(int i = soffs; i < len; i++) {
    if(bits_per_half_wavelength - bit_number > 8) {
        buff[i] = bit_state ? 0xFF : 0x00;
        bit_number += 8;
        if(bit_number >= bits_per_half_wavelength) {
            bit_number -= bits_per_half_wavelength;
            bit_state = !bit_state;
        }
    } else 
        for(int j = 0; j < 8; j++) {
            bit_number++;
            if(bit_state) {
                buff[i] |= (1 << j);
            } 
            if(bit_number >= bits_per_half_wavelength) {
                bit_number -= bits_per_half_wavelength;
                bit_state = !bit_state;
            }
        }
   }
   ESP_LOGI(LOG_TAG,"[0] = %x, [1] = %x", buff[0], buff[1]);
}


static void oelutz() {
    ESP_LOGI(LOG_TAG, "oelu");
    esp_err_t e;
     // Begin an SPI transaction
    spi_transaction_t t;
    t.flags = 0;              // no flags
    t.length = 8 * max_size;  // set Tx size in bits
    t.tx_buffer = dma_buffer; // Tx from DMA buffer
    t.rx_buffer = nullptr;    // Rx to nowhere
    t.rxlength = 0;           // don't Rx at all
    
    // Request exclusive SPI access for timing precision
    e = spi_device_acquire_bus(handle, portMAX_DELAY);
    if (e != ESP_OK) {
        ESP_LOGE(LOG_TAG, "[ERROR] SPI device lock failed : %d\n", e);
    } 
    
    // Send data over SPI interface
    e = spi_device_transmit(handle, &t);
    if (e != ESP_OK) {
        ESP_LOGE(LOG_TAG, "[ERROR] SPI device transmit failed : %d\n", e);
    } else {
        ESP_LOGV(LOG_TAG, "SPI sent %d bytes", got_size);
    }

    // Release exclusive access of SPI bus
    spi_device_release_bus(handle);
}

void ass() {
 // Use MOSI at pager pin, don't use other pins
    bus_cfg.sclk_io_num = -1;
    bus_cfg.miso_io_num = -1;
    bus_cfg.mosi_io_num = HWCONF_BEEPER_GPIO;
    if_cfg.spics_io_num = -1; //<- CS pin is selected at interface level

    // Set max transfer size in bytes
    bus_cfg.max_transfer_sz = max_size;
    
    // Suitable configuration found experimentally
    if_cfg.mode = mode;
    if_cfg.clock_speed_hz = baud;
    if_cfg.queue_size = 1;
    if_cfg.flags = SPI_DEVICE_NO_DUMMY;
    if_cfg.pre_cb = NULL;
    if_cfg.post_cb = NULL;
    if_cfg.dummy_bits = 0;
    if_cfg.command_bits = 0;
    if_cfg.address_bits = 0;
    
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
    
    // Make a DMA buffer for the messages output
    dma_buffer = heap_caps_calloc(1, max_size, MALLOC_CAP_DMA);

    while(1) {
        memset(dma_buffer, 0, max_size);
        freq_to_buff(523);
        oelutz();
        vTaskDelay(pdMS_TO_TICKS(250));

        memset(dma_buffer, 0, max_size);
        freq_to_buff(587);
        oelutz();
        vTaskDelay(pdMS_TO_TICKS(250));

        memset(dma_buffer, 0, max_size);
        freq_to_buff(698);
        oelutz();
        vTaskDelay(pdMS_TO_TICKS(500));

        memset(dma_buffer, 0, max_size);
        freq_to_buff(523);
        freq_to_buff(2200);
        oelutz();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}