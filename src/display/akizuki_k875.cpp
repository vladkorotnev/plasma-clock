#include <device_config.h>

#if HAS(OUTPUT_AKIZUKI_K875)
#include "display/akizuki_k875.h"
#include "Arduino.h"

static char LOG_TAG[] = "SWEEP";

// Pretty crappy code quality converted from a PoC
// But it seems to work!
// A much cleaner solution would be to use a circular transaction, such as: https://github.com/t-mat/esp32-vga-experiment/blob/master/main/my_spi.c
// But I don't want to mess around that much for now!
// Known issue: if the CPU freezes the framebuffer task or something, the screen blanks for a bit

void* MAGIC_TXN_ID = (void*) 0x39393939;
const uint8_t matrices_per_panel = 2;
const uint8_t bus_cycles_per_panel = 16;
const uint8_t bus_cycles_per_byte = 2;
const uint8_t rows = 16;
const int columns_per_panel = 32;

static spi_transaction_t trans = {
    .flags = SPI_TRANS_MODE_QIO,
    .cmd = 0,
    .addr = 0,
    .length = 0, //<- to be set
    .rxlength = 0,
    .user = MAGIC_TXN_ID,
    .tx_buffer = nullptr, //<- to be set
    .rx_data = { 0 }
};
static gpio_num_t _latch = GPIO_NUM_0;
static uint8_t * _data = nullptr;
static int _row = 0;
static spi_device_handle_t hDev = nullptr;  
static size_t _bytes_per_row = 0;
static spi_transaction_t * _txn = nullptr;

IRAM_ATTR void txn_done_isr(spi_transaction_t * txn) {
    if(txn->user != MAGIC_TXN_ID || _latch == 0 || _data == nullptr || _bytes_per_row == 0 || _txn == nullptr) return;
    digitalWrite(_latch, 0);
    digitalWrite(_latch, 1);
}

AkizukiK875Driver::AkizukiK875Driver(
    gpio_num_t latch_pin,
    gpio_num_t clock_pin,
    gpio_num_t strobe_pin,
    gpio_num_t sin1_pin,
    gpio_num_t sin2_pin,
    gpio_num_t sin3_pin,
    gpio_num_t sacrificial_pin,
    uint8_t ledc_channel,
    int bright_pwm,
    int dark_pwm,
    uint8_t panel_count,
    uint8_t desired_frame_clock,
    spi_host_device_t host
):
    LATCH_PIN(latch_pin),
    CLOCK_PIN(clock_pin),
    SIN1_PIN(sin1_pin),
    SIN2_PIN(sin2_pin),
    SIN3_PIN(sin3_pin),
    STROBE_PIN(strobe_pin),
    SACRIFICIAL_UNUSE_PIN(sacrificial_pin),
    PANEL_COUNT(panel_count),
    PIXEL_CLOCK_HZ(desired_frame_clock * rows * panel_count * columns_per_panel),
    total_bytes_per_row(bus_cycles_per_panel * panel_count / bus_cycles_per_byte),
    ledcChannel(ledc_channel),
    brightPwm(bright_pwm),
    darkPwm(dark_pwm),
    spi(host)
{
    assert(HWCONF_DISPLAY_HEIGHT_PX == rows);
    assert(HWCONF_DISPLAY_WIDTH_PX == PANEL_COUNT * columns_per_panel);
}

void AkizukiK875Driver::initialize() {
    data = (uint8_t*) heap_caps_calloc(1, total_bytes_per_row * (rows+1), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if(data == nullptr) {
        ESP_LOGE(LOG_TAG, "Could not allocate data buffer! Wanted %i bytes", total_bytes_per_row * rows);
        return;
    }
    trans.length = total_bytes_per_row * 8;
    trans.tx_buffer = data;
    _txn = &trans;
    _row = 0;
    _data = data;
    _latch = LATCH_PIN;
    _bytes_per_row = total_bytes_per_row;

    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, 1);

    ledcSetup(ledcChannel, 16000, 8);
    ledcAttachPin(STROBE_PIN, ledcChannel);
    ledcWrite(ledcChannel, brightPwm);
    curPwm = brightPwm;
    targetPwm = brightPwm;

    spi_bus_config_t bus_cfg = {
        .data0_io_num = SIN1_PIN, // SIN1
        .data1_io_num = SIN2_PIN, // SIN2
        .sclk_io_num = CLOCK_PIN, // COCK
        .data2_io_num = SIN3_PIN, // SIN3
        .data3_io_num = SACRIFICIAL_UNUSE_PIN, // unuse for now, sacrificial pin
        .data4_io_num = -1, // unuse for now
        .data5_io_num = -1, // unuse for now
        .data6_io_num = -1, // unuse for now
        .data7_io_num = -1, // unuse for now
        .max_transfer_sz = 0,
        .flags = SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_QUAD | SPICOMMON_BUSFLAG_GPIO_PINS
    };

    esp_err_t res = spi_bus_initialize(
        spi,
        &bus_cfg,
        SPI_DMA_CH_AUTO
    );

    if(res != ESP_OK) {
        ESP_LOGE(LOG_TAG, "SPI Init Error %i: %s", res, esp_err_to_name(res));
        return;
    }
    else {
        spi_device_interface_config_t dev_cfg = {
            .command_bits = 0,
            .address_bits = 0,
            .dummy_bits = 0,
            .mode = 3,
            .duty_cycle_pos = 0,
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = PIXEL_CLOCK_HZ,
            .input_delay_ns = 0,
            .spics_io_num = -1,
            .flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_BIT_LSBFIRST | SPI_DEVICE_POSITIVE_CS,
            .queue_size = 128,
            .pre_cb = nullptr,
            .post_cb = txn_done_isr
        };

        res = spi_bus_add_device(spi, &dev_cfg, &hDev);
        if(res != ESP_OK) {
            ESP_LOGE(LOG_TAG, "SPI Dev Init Error %i: %s", res, esp_err_to_name(res));
            return;
        }
    }

    res = spi_device_queue_trans(hDev, &trans, portMAX_DELAY);
    if(res != ESP_OK) ESP_LOGE(LOG_TAG, "SPI Dev Txn Error %i: %s", res, esp_err_to_name(res));
}

void AkizukiK875Driver::write_fanta(const uint8_t * strides, size_t count) {
    const uint16_t * columns = (const uint16_t*) strides;
    for(int row = 0; row < rows; row++) {
        uint8_t *row_array = &data[row * total_bytes_per_row];
        memset(row_array, 0, total_bytes_per_row);
        for(int col_idx = 0; col_idx < total_bytes_per_row * 2; col_idx++) {
            size_t byte_idx = col_idx / 2;
            uint8_t nibble_idx = col_idx % 2;
            int index1 = (count / 2) - (((col_idx / (columns_per_panel / 2)) * columns_per_panel) + (col_idx % (columns_per_panel / 2))) - 1;
            int index2 = (count / 2) - (((col_idx / (columns_per_panel / 2)) * columns_per_panel) + ((columns_per_panel / 2) + (col_idx % (columns_per_panel / 2)))) - 1;
            uint16_t led2 = columns[index1];
            uint16_t led1 = columns[index2];

            row_array[byte_idx] |= 
                (
                    ((led1 & (1 << row)) == 0 ? 0 : 0b010) |
                    ((led2 & (1 << row)) == 0 ? 0 : 0b100)
                ) << (nibble_idx ? 4 : 0);
        }
        // put the row enable signals at appropriate locations
        uint8_t byte_no = row / 2;
        uint8_t nibble_no = row % 2;
        for(int i = 0; i < PANEL_COUNT; i++) {
            row_array[byte_no + i * bus_cycles_per_panel / bus_cycles_per_byte] |= 0b001 << (nibble_no == 0 ? 0 : 4);
        }
        _txn->tx_buffer = row_array;
        esp_err_t res = spi_device_queue_trans(hDev, _txn, portMAX_DELAY);
        if(res != ESP_OK) ESP_LOGE(LOG_TAG, "SPI Dev Txn Error %i: %s", res, esp_err_to_name(res));
    }

    _txn->tx_buffer = &data[rows * total_bytes_per_row]; // dummy empty row to prevent a hotspot between frame renders
    esp_err_t res = spi_device_queue_trans(hDev, _txn, portMAX_DELAY);
    if(res != ESP_OK) ESP_LOGE(LOG_TAG, "SPI Dev Txn Error %i: %s", res, esp_err_to_name(res));

    if(curPwm != targetPwm) {
        if(curPwm < targetPwm) {
            curPwm = std::min(curPwm + 10, targetPwm);
        } else {
            curPwm = std::max(curPwm - 10, targetPwm);
        }
        ledcWrite(ledcChannel, curPwm);
    }
}

void AkizukiK875Driver::set_power(bool power) {
    power_state = power;
    update_strobe_value();
}

void AkizukiK875Driver::set_show(bool show) {
    show_state = show;
    update_strobe_value();
}

void AkizukiK875Driver::set_bright(bool bright) {
    bright_state = bright;
    update_strobe_value();
}

void AkizukiK875Driver::update_strobe_value() {
    ESP_LOGI(LOG_TAG, "Power=%i, Show=%i, Bright=%i", power_state, show_state, bright_state);
    if(!power_state || !show_state) {
        targetPwm = 255;
    } 
    else {
        if(bright_state) {
            targetPwm = brightPwm;
        } else {
            targetPwm = darkPwm;
        }
    }
}

#endif