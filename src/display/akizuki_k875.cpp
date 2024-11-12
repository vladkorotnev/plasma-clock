#include <device_config.h>

// Well, hello my friend.
// I see you have decided to use a bunch of crossed over shift registers as a display.
//
// Sadly, doing that in any sustainable capacity without clogging the CPU with interrupts or queueing transactions is hard.
// But see, this is not a production product, this is somewhat demoscene. So I did it anyway.
//
// Things might break horribly depending on your silicon rev. or SDK version. Dragons lie ahead, or fly, or walk, or idk, it's hard to move when
// you have so much of them stuffed inside a 1.5cm2 chip.
//
// Welcome to hell.

#if HAS(OUTPUT_AKIZUKI_K875)

// Akizuki K875 Display Driver

#include <hal/spi_ll.h>
#define SPI_LL_GET_CLK(ID) ((ID)==0? SPICLK_OUT_IDX:((ID)==1? HSPICLK_OUT_IDX : VSPICLK_OUT_IDX))
#define SPI_LL_GET_HD(ID) ((ID)==0? SPIHD_OUT_IDX:((ID)==1? HSPIHD_OUT_IDX : VSPIHD_OUT_IDX))

#include <hal/mcpwm_ll.h>
#include <driver/mcpwm.h>
#include <driver/ledc.h>
#include <soc/spi_reg.h>
#include <soc/dport_reg.h>
#include <soc/mcpwm_reg.h>
#include "display/akizuki_k875.h"
#include "Arduino.h"

static char LOG_TAG[] = "SWEEP";
static portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

/// @brief LED matrices (16x16) per panel
const uint8_t matrices_per_panel = 2;
/// @brief QIO bus cycle count to fully draw one panel
const uint8_t bus_cycles_per_panel = 16;
/// @brief Bus cycles per one data byte (QIO == 1 cycle per nibble, 2 cycle per byte)
const uint8_t bus_cycles_per_byte = 2;
/// @brief Rows in panels (always 16)
const uint8_t rows = 16;
/// @brief Columns in panels (2x16)
const int columns_per_panel = 32;

/// Bit value that corresponds to SIN1 (vertical data) on the QIO bus
#define QIO_BITVAL_SIN1  0b0001
/// Bit value that corresponds to the SIN2 (horizontal data LED1) on the QIO bus
#define QIO_BITVAL_SIN2  0b0010
/// Bit value that corresponds to the SIN3 (horizontal data LED2) on the QIO bus
#define QIO_BITVAL_SIN3  0b0100
/// Bit value that corresponds to the LATCH on the QIO bus
#define QIO_BITVAL_LATCH 0b1000

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
    PIXEL_CLOCK_HZ(250000),
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
    ESP_LOGI(LOG_TAG, "Expected pixel clock = %i", PIXEL_CLOCK_HZ);

    // Create DMA-accessible buffer for the signals
    dma_buffer = (uint8_t*) heap_caps_calloc(1, (total_bytes_per_row + 1) * rows, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    if(dma_buffer == nullptr) {
        ESP_LOGE(LOG_TAG, "Could not allocate data buffer! Wanted %i bytes", (total_bytes_per_row + 1) * rows);
        return;
    }

    // On average it takes about 400us to transform the data from Fanta format into the signal sequence format, but only 3us to copy within the internal memory
    // So to reduce flickering due to DMA concurrent access, let's prep the data in a scratch buffer then copy it
    scratch_buffer = (uint8_t*) heap_caps_calloc(1, (total_bytes_per_row + 1) * rows, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    if(scratch_buffer == nullptr) {
        ESP_LOGE(LOG_TAG, "Could not allocate scratch buffer! Wanted %i bytes", (total_bytes_per_row + 1) * rows);
        return;
    }

    // Setup LEDC for brightness control
    ledcSetup(ledcChannel, 16000, 8);
    ledcAttachPin(STROBE_PIN, ledcChannel);
    ledcWrite(ledcChannel, brightPwm);
    curPwm = brightPwm;
    targetPwm = brightPwm;

    // Dummy transaction to prep the SPI
    spi_transaction_t trans = {
        .flags = SPI_TRANS_MODE_QIO,
        .cmd = 0,
        .addr = 0,
        .length = total_bytes_per_row * 8,
        .rxlength = 0,
        .user = nullptr,
        .tx_buffer = dma_buffer,
        .rx_data = { 0 }
    };

    // Prep the SPI bus as usual, but use a temp pin for SCK
    spi_bus_config_t bus_cfg = {
        .data0_io_num = SIN1_PIN, // SIN1
        .data1_io_num = SIN2_PIN, // SIN2
        .sclk_io_num = SACRIFICIAL_UNUSE_PIN, // temp
        .data2_io_num = SIN3_PIN, // SIN3
        .data3_io_num = LATCH_PIN, // LATCH
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
        // Prep an SPI device handle as usual. Since in DMA mode we will have CS constantly asserted, there is no point in specifying a pin.

        spi_device_handle_t hDev = nullptr;  
        spi_device_interface_config_t dev_cfg = {
            .command_bits = 0,
            .address_bits = 0,
            .dummy_bits = 0,
            .mode = 0,
            .duty_cycle_pos = 0,
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = PIXEL_CLOCK_HZ,
            .input_delay_ns = 0,
            .spics_io_num = -1,
            .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_BIT_LSBFIRST,
            .queue_size = 4,
            .pre_cb = nullptr,
            .post_cb = nullptr
        };

        res = spi_bus_add_device(spi, &dev_cfg, &hDev);
        if(res != ESP_OK) {
            ESP_LOGE(LOG_TAG, "SPI Dev Init Error %i: %s", res, esp_err_to_name(res));
            return;
        }

        // Do a dummy transaction to prime all the SPI registers
        res = spi_device_queue_trans(hDev, &trans, portMAX_DELAY);
        if(res != ESP_OK) ESP_LOGE(LOG_TAG, "SPI Dev Txn Error %i: %s", res, esp_err_to_name(res));
        else {
            // Now here starts the fun stuff part 1... but first wait for the dummy txn to finish.
            spi_transaction_t * t = &trans;
            res = spi_device_get_trans_result(hDev, &t, portMAX_DELAY);
            if(res != ESP_OK) ESP_LOGE(LOG_TAG, "SPI Dev Wait Error %i: %s", res, esp_err_to_name(res));

            // And prime the data buffer with some pattern to indicate init succeeded
            for(int row = 0; row < rows; row++) {
                uint8_t *row_array = &dma_buffer[row * (total_bytes_per_row + 1)];
                memset(row_array, 0, (total_bytes_per_row + 1));
                for(int col_idx = 0; col_idx < total_bytes_per_row * 2; col_idx++) {
                    size_t byte_idx = col_idx / 2;
                    uint8_t nibble_idx = col_idx % 2;

                    row_array[byte_idx] |= 
                        (
                            ((row % 2) == 0 && (col_idx % 2 == 0) ? QIO_BITVAL_SIN2 : 0) |
                            (((row+1) % 2) == 0 && (col_idx % 2 == 0) ? QIO_BITVAL_SIN3 : 0) |
                            QIO_BITVAL_LATCH // <- close the LATCH at all times
                        ) << (nibble_idx ? 4 : 0);
                }

                // put the row enable signals at appropriate locations
                uint8_t byte_no = row / 2;
                uint8_t nibble_no = row % 2;
                for(int i = 0; i < PANEL_COUNT; i++) {
                    row_array[byte_no + i * bus_cycles_per_panel / bus_cycles_per_byte] |= QIO_BITVAL_SIN1 << (nibble_no == 0 ? 0 : 4);
                }

                row_array[total_bytes_per_row] = (QIO_BITVAL_LATCH << 4); // open the LATCH for one cycle at the end of the row
            }

            // Get the raw SPI hardware register pointer
            spi_dev_t* const spiHw = SPI_LL_GET_HW(spi);

            // Allocate the DMA linked-list descriptors
            lldesc_s * lldescs = (lldesc_s*) heap_caps_calloc(1, sizeof(lldesc_s) * rows, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

            for(int i = 0; i < rows; i++) {
                lldesc_s * d = &lldescs[i];
                // Put a single row of display image plus the LATCH signal into the linked list entry
                d->buf = &dma_buffer[i * (total_bytes_per_row + 1)];
                d->length = total_bytes_per_row + 1;
                d->size = total_bytes_per_row + 1;
                d->owner = LLDESC_HW_OWNED;
                // Make the linked list circular so that the transmission never ends
                if(i == rows - 1) {
                    d->qe.stqe_next = &lldescs[0];
                } else {
                    d->qe.stqe_next = &lldescs[i + 1];
                }
            }

            // Since the SIO/DIO/QIO modes are associated with the device handle in the driver, set the QIO mode directly in hardware
            spi_line_mode_t lm = {
                .cmd_lines = 1,
                .addr_lines = 1,
                .data_lines = 4,
            };
            spi_ll_master_set_line_mode(spiHw, lm);

            // And now for the most cursed part...
            // Because we've put the LATCH signal inside our data array, even when we open the latch to allow the LEDs to glow, the SPI clock doesn't stop.
            // And due to the endless SPI transmission we don't have any CS line or similar too.
            // As a result, the image on the screen is either shifted 1px up and right, or otherwise blurred or distorted, because the shift registers are continuing
            // to shift even though the latch is open.
            //
            // To properly go around this we need to clock the display with something like DISP_CLOCK = SPI_CLOCK && LATCH, so when the LATCH is 0 it "mutes" the clock.
            // One viable approach would be to solder a 74-series AND or NAND gate in the circuit, but I'm too lazy and cheap to use a whole package for just 2 transistors worth of stuff.
            // And I'm even lazier to use just 2 transistors, because discrete is where you start getting all sorts of drift and headache.
            // However, the ESP32 has a very flexible Pin Mux grid, and also some fun peripherals, such as the Motor Control PWM unit.
            // See where this is going?

            // Allocate a MCPWM unit at 500kHz (somehow more doesn't work for me).
            // Set duty cycle to 100% (a.k.a. constant logic HIGH)
            mcpwm_config_t mcpwm_config = {
                .frequency = 500000,
                .cmpr_a = 100.0,
                .cmpr_b = 100.0,
                .duty_mode = MCPWM_DUTY_MODE_0,
                .counter_mode = MCPWM_UP_COUNTER,
            };
            ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &mcpwm_config));
            ESP_ERROR_CHECK(mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0));

            // Create two emergency stop inputs on the MCPWM unit...
            ESP_ERROR_CHECK(mcpwm_fault_init(MCPWM_UNIT_0, MCPWM_LOW_LEVEL_TGR, MCPWM_SELECT_F0));
            ESP_ERROR_CHECK(mcpwm_fault_init(MCPWM_UNIT_0, MCPWM_LOW_LEVEL_TGR, MCPWM_SELECT_F1));
            // ...and tell it to force to logic LOW whenever either of them goes LOW.
            ESP_ERROR_CHECK(mcpwm_fault_set_cyc_mode(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_F0, MCPWM_ACTION_FORCE_LOW, MCPWM_ACTION_FORCE_LOW));
            ESP_ERROR_CHECK(mcpwm_fault_set_cyc_mode(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_F1, MCPWM_ACTION_FORCE_LOW, MCPWM_ACTION_FORCE_LOW));
            // Thus we have just created an AND gate out of the MCPWM unit!
            ESP_ERROR_CHECK(mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0));

            // The emergency stop interrupts will clog the heck out of our CPU, so let's get rid of that before too late.
            mcpwm_ll_intr_disable_all(&MCPWM0);

            // Assign the pins to the GPIO mux, specifically the DISP_CLOCK pin, the LATCH pin, and the SACRIFICIAL pin
            PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[CLOCK_PIN], PIN_FUNC_GPIO);
            PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[SACRIFICIAL_UNUSE_PIN], PIN_FUNC_GPIO);
            PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[LATCH_PIN], PIN_FUNC_GPIO);
            // Set them all as bidirectional
            gpio_set_direction(CLOCK_PIN, GPIO_MODE_INPUT_OUTPUT);
            gpio_set_direction(SACRIFICIAL_UNUSE_PIN, GPIO_MODE_INPUT_OUTPUT);
            gpio_set_direction(LATCH_PIN, GPIO_MODE_INPUT_OUTPUT);

            // Connect the sacrificial pin as follows:
            //      [SPI]CLK -------->  SACRIFICIAL PIN --------> FAULT0[MCPWM]
            gpio_matrix_out(SACRIFICIAL_UNUSE_PIN, SPI_LL_GET_CLK(spi), false, false);
            gpio_matrix_in(SACRIFICIAL_UNUSE_PIN, PWM0_F0_IN_IDX, false);

            // Connect the DISPLAY CLOCK pin as follows:
            //      [MCPWM]OUT0A -------->  CLOCK
            gpio_matrix_out(CLOCK_PIN, PWM0_OUT0A_IDX, false, false);

            // Connect the LATCH pin as follows:
            //      [SPI]HD(QDATA3) --------> LATCH -------->  FAULT1[MCPWM]
            gpio_matrix_out(LATCH_PIN, SPI_LL_GET_HD(spi), false, false);
            gpio_matrix_in(LATCH_PIN, PWM0_F1_IN_IDX, false);

            // The resulting connection diagram is as follows:
            //
            //  [SPI]CLK o-------\           MCPWM
            //                    \    +----------------+
            //                      ---| F0             |
            //                         |             OUT|---------o DISP_CLOCK = (F0 && F1) = (CLK & LATCH)
            //                      ---| F1             |
            //                     /   +----------------+
            //  [SPI]HD(QD3) o----/
            //
            // Mission accomplished!!

            // Point SPI peripheral to our DMA linked list
            spiHw->dma_out_link.addr           = (int)(lldescs) & 0xFFFFF;

            // Set circular mode
            //      https://www.esp32.com/viewtopic.php?f=2&t=4011#p18107
            //      > yes, in SPI DMA mode, SPI will alway transmit and receive
            //      > data when you set the SPI_DMA_CONTINUE(BIT16) of SPI_DMA_CONF_REG.
            spiHw->dma_conf.dma_tx_stop		= 0;	// Disable stop
            spiHw->dma_conf.dma_continue	= 1;	// Set contiguous mode
            spiHw->dma_out_link.start		= 1;	// Start SPI DMA transfer (1)

            // And off we go! Dynamic scan on shift registers without any CPU time whatsoever!
            spiHw->cmd.usr = 1;

            // Now the display should be showing things.
            // If you are still here, it's time to go buy a proper display controller and spend your life on something better than this file.
            // Or buy a beer or coke or juice and continue. No pressure whatsoever.
        }
    }
}

void AkizukiK875Driver::write_fanta(const uint8_t * strides, size_t count) {
    // Prep the signal sequence: 400us on average
    // Might be possible to optimize but eh
    const uint16_t * columns = (const uint16_t*) strides;
    for(int row = 0; row < rows; row++) {
        uint8_t *row_array = &scratch_buffer[row * (total_bytes_per_row + 1)];
        memset(row_array, 0, (total_bytes_per_row + 1));
        for(int col_idx = 0; col_idx < total_bytes_per_row * 2; col_idx++) {
            size_t byte_idx = col_idx / 2;
            uint8_t nibble_idx = col_idx % 2;
            int index1 = (count / 2) - (((col_idx / (columns_per_panel / 2)) * columns_per_panel) + (col_idx % (columns_per_panel / 2))) - 1;
            int index2 = (count / 2) - (((col_idx / (columns_per_panel / 2)) * columns_per_panel) + ((columns_per_panel / 2) + (col_idx % (columns_per_panel / 2)))) - 1;
            uint16_t led2 = columns[index1];
            uint16_t led1 = columns[index2];

            row_array[byte_idx] |= 
                (
                    ((led1 & (1 << row)) == 0 ? 0 : QIO_BITVAL_SIN2) |
                    ((led2 & (1 << row)) == 0 ? 0 : QIO_BITVAL_SIN3) |
                    QIO_BITVAL_LATCH // <- close the LATCH
                ) << (nibble_idx ? 4 : 0);
        }
        // put the row enable signals at appropriate locations
        uint8_t byte_no = row / 2;
        uint8_t nibble_no = row % 2;
        for(int i = 0; i < PANEL_COUNT; i++) {
            row_array[byte_no + i * bus_cycles_per_panel / bus_cycles_per_byte] |= QIO_BITVAL_SIN1 << (nibble_no == 0 ? 0 : 4);
        }
        row_array[total_bytes_per_row] = (QIO_BITVAL_LATCH << 4); // open the LATCH for one cycle at the end of the row
    }

    taskENTER_CRITICAL(&_spinlock);
    // Copy from the scratch buffer into DMA buffer: 3.5us on average
    memcpy(dma_buffer, scratch_buffer, (total_bytes_per_row + 1) * rows);
    taskEXIT_CRITICAL(&_spinlock);

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