#include <device_config.h>

#if HAS(OUTPUT_AKIZUKI_K875)
#include "display/akizuki_k875.h"
#include "Arduino.h"
#include <soc/spi_reg.h>
#include <soc/dport_reg.h>

static char LOG_TAG[] = "K875";

// Pretty crappy code quality converted from a PoC
// But it seems to work!
// A much cleaner solution would be to use a circular transaction, such as: https://github.com/t-mat/esp32-vga-experiment/blob/master/main/my_spi.c
// But I don't want to mess around that much for now!
// Known issue: if the CPU freezes the framebuffer task or something, the screen blanks for a bit

const uint8_t matrices_per_panel = 2;
const uint8_t bus_cycles_per_panel = 16;
const uint8_t bus_cycles_per_byte = 2;
const uint8_t rows = 16;
const int columns_per_panel = 32;
static uint8_t * _data = nullptr;
static int _row = 0;
static size_t _bytes_per_row = 0;

static const uint8_t InvalidIndex = (uint8_t) -1;

spi_dev_t *myspi_get_hw_for_host(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return &SPI1; break;
    case HSPI_HOST: return &SPI2; break;
    case VSPI_HOST: return &SPI3; break;
    default:        return NULL;  break;
    }
}

static uint8_t getSpidOutByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPID_OUT_IDX;	break;
    case HSPI_HOST: return HSPID_OUT_IDX;	break;
    case VSPI_HOST: return VSPID_OUT_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpidInByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPID_IN_IDX;		break;
    case HSPI_HOST: return HSPID_IN_IDX;	break;
    case VSPI_HOST: return VSPID_IN_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiqOutByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIQ_OUT_IDX;	break;
    case HSPI_HOST: return HSPIQ_OUT_IDX;	break;
    case VSPI_HOST: return VSPIQ_OUT_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiqInByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIQ_IN_IDX;		break;
    case HSPI_HOST: return HSPIQ_IN_IDX;	break;
    case VSPI_HOST: return VSPIQ_IN_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiWpOutByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIWP_OUT_IDX;	break;
    case HSPI_HOST: return HSPIWP_OUT_IDX;	break;
    case VSPI_HOST: return VSPIWP_OUT_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiWpInByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIWP_IN_IDX;		break;
    case HSPI_HOST: return HSPIWP_IN_IDX;	break;
    case VSPI_HOST: return VSPIWP_IN_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiHdOutByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIHD_OUT_IDX;	break;
    case HSPI_HOST: return HSPIHD_OUT_IDX;	break;
    case VSPI_HOST: return VSPIHD_OUT_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getSpiHdInByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPIHD_IN_IDX;		break;
    case HSPI_HOST: return HSPIHD_IN_IDX;	break;
    case VSPI_HOST: return VSPIHD_IN_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getCsOutByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPICS0_OUT_IDX;	break;
    case HSPI_HOST: return HSPICS0_OUT_IDX;	break;
    case VSPI_HOST: return VSPICS0_OUT_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

static uint8_t getCsInByHost(
    spi_host_device_t host
) {
    switch(host) {
    case SPI_HOST:  return SPICS0_IN_IDX;	break;
    case HSPI_HOST: return HSPICS0_IN_IDX;	break;
    case VSPI_HOST: return VSPICS0_IN_IDX;	break;
    default:        return InvalidIndex;	break;
    }
}

esp_err_t myspi_prepare_circular_buffer(
      const spi_host_device_t   spiHostDevice
    // , const int                 dma_chan
    , const lldesc_t*           lldescs
    , const double              dmaClockSpeedInHz
    , const gpio_num_t          data0_gpio_num // mosi (spid)
    , const gpio_num_t          data1_gpio_num // miso (spiq)
    , const gpio_num_t          data2_gpio_num // spiwp
    , const gpio_num_t          data3_gpio_num // spihd
    , const gpio_num_t          cs_gpio_num
    , const int                 waitCycle = 0
) {
    spi_dev_t* const spiHw = myspi_get_hw_for_host(spiHostDevice);
    const int Cs = 0;
	const int CsMask = 1 << Cs;

    //Use GPIO
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[data0_gpio_num], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[data1_gpio_num], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[data2_gpio_num], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[data3_gpio_num], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[cs_gpio_num], PIN_FUNC_GPIO);

    gpio_set_direction(data0_gpio_num, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(data1_gpio_num, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(data2_gpio_num, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(data3_gpio_num, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(cs_gpio_num, GPIO_MODE_INPUT_OUTPUT);

    gpio_matrix_out(data0_gpio_num, getSpidOutByHost(spiHostDevice), false, false);
    gpio_matrix_out(data1_gpio_num, getSpiqOutByHost(spiHostDevice), false, false);
    gpio_matrix_out(data2_gpio_num, getSpiWpOutByHost(spiHostDevice), false, false);
    gpio_matrix_out(data3_gpio_num, getSpiHdOutByHost(spiHostDevice), false, false);
    gpio_matrix_out(cs_gpio_num, getCsOutByHost(spiHostDevice), false, false);

    gpio_matrix_in(data0_gpio_num, getSpidInByHost(spiHostDevice), false);
    gpio_matrix_in(data1_gpio_num, getSpiqInByHost(spiHostDevice), false);
    gpio_matrix_in(data2_gpio_num, getSpiWpInByHost(spiHostDevice), false);
    gpio_matrix_in(data3_gpio_num, getSpiHdInByHost(spiHostDevice), false);
    gpio_matrix_in(cs_gpio_num, getCsInByHost(spiHostDevice), false);

    //Select DMA channel.
    // DPORT_SET_PERI_REG_BITS(
    //       DPORT_SPI_DMA_CHAN_SEL_REG
    //     , 3
    //     , dma_chan
    //     , (spiHostDevice * 2)
    // );

    //Reset DMA
    spiHw->dma_conf.val        		|= SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST;
    spiHw->dma_out_link.start  		= 0;
    spiHw->dma_in_link.start   		= 0;
    spiHw->dma_conf.val        		&= ~(SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST);

    //Reset timing
    spiHw->ctrl2.val           		= 0;

    //Disable unneeded ints
    spiHw->slave.rd_buf_done   		= 0;
    spiHw->slave.wr_buf_done   		= 0;
    spiHw->slave.rd_sta_done   		= 0;
    spiHw->slave.wr_sta_done   		= 0;
    spiHw->slave.rd_buf_inten  		= 0;
    spiHw->slave.wr_buf_inten  		= 0;
    spiHw->slave.rd_sta_inten  		= 0;
    spiHw->slave.wr_sta_inten  		= 0;
    spiHw->slave.trans_inten   		= 0;
    spiHw->slave.trans_done    		= 0;

    //Set CS pin, CS options
	spiHw->pin.master_ck_sel			&= ~CsMask;
	spiHw->pin.master_cs_pol			&= ~CsMask; // <- for LATCH high during output //&= ~CsMask;

	// Set SPI Clock
	//  Register 7.7: SPI_CLOCK_REG (0x18)
	//
	//		SPI_CLK_EQU_SYSCLK
	//			In master mode, when this bit is set to 1, spi_clk is equal
	//			to system clock; when set to 0, spi_clk is divided from system
	//			clock.
	//
	//		SPI_CLKDIV_PRE
	//			In master mode, the value of this register field is the
	//			pre-divider value for spi_clk, minus one.
	//
	//		SPI_CLKCNT_N
	//			In master mode, this is the divider for spi_clk minus one.
	//			The spi_clk frequency is
	//				system_clock/(SPI_CLKDIV_PRE+1)/(SPI_CLKCNT_N+1).
	//
	//		SPI_CLKCNT_H
	//			For a 50% duty cycle, set this to floor((SPI_CLKCNT_N+1)/2-1)
	//
	//		SPI_CLKCNT_L
	//			In master mode, this must be equal to SPI_CLKCNT_N.
	{
		const double	preDivider			= 1.0;
	    const double	apbClockSpeedInHz	= APB_CLK_FREQ;
		const double	apbClockPerDmaCycle	= (apbClockSpeedInHz / preDivider / dmaClockSpeedInHz);

		const int32_t	clkdiv_pre	= ((int32_t) preDivider) - 1;
		const int32_t	clkcnt_n	= ((int32_t) apbClockPerDmaCycle) - 1;
		const int32_t	clkcnt_h	= (clkcnt_n + 1) / 2 - 1;
		const int32_t	clkcnt_l	= clkcnt_n;

		spiHw->clock.clk_equ_sysclk	= 0;
	    spiHw->clock.clkcnt_n		= clkcnt_n;
	    spiHw->clock.clkdiv_pre		= clkdiv_pre;
		spiHw->clock.clkcnt_h		= clkcnt_h;
	    spiHw->clock.clkcnt_l		= clkcnt_l;
	}

    //Configure bit order
    spiHw->ctrl.rd_bit_order           = 0;    // MSB first
    spiHw->ctrl.wr_bit_order           = 1;    // LSB first

    //Configure polarity
    spiHw->pin.ck_idle_edge            = 0;
    spiHw->user.ck_out_edge            = 0;
    spiHw->ctrl2.miso_delay_mode       = 0;

    //configure dummy bits
    spiHw->user.usr_dummy              = 0;
    spiHw->user1.usr_dummy_cyclelen    = 0;

    //Configure misc stuff
    spiHw->user.doutdin                = 0;
    spiHw->user.sio                    = 0;

    spiHw->ctrl2.setup_time            = 0;
    spiHw->user.cs_setup               = 0;
    spiHw->ctrl2.hold_time             = 0;
    spiHw->user.cs_hold                = 0;

    //Configure CS pin
    spiHw->pin.cs0_dis                 = (Cs == 0) ? 0 : 1;
    spiHw->pin.cs1_dis                 = (Cs == 1) ? 0 : 1;
    spiHw->pin.cs2_dis                 = (Cs == 2) ? 0 : 1;

    //Reset SPI peripheral
    spiHw->dma_conf.val                |= SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST;
    spiHw->dma_out_link.start          = 0;
    spiHw->dma_in_link.start           = 0;
    spiHw->dma_conf.val                &= ~(SPI_OUT_RST|SPI_IN_RST|SPI_AHBM_RST|SPI_AHBM_FIFO_RST);
    spiHw->dma_conf.out_data_burst_en  = 1;

    //Set up QIO
    spiHw->ctrl.val		&= ~(SPI_FREAD_DUAL|SPI_FREAD_QUAD|SPI_FREAD_DIO|SPI_FREAD_QIO);
    spiHw->user.val		&= ~(SPI_FWRITE_DUAL|SPI_FWRITE_QUAD|SPI_FWRITE_DIO|SPI_FWRITE_QIO);

    //DMA temporary workaround: let RX DMA work somehow to avoid the issue in ESP32 v0/v1 silicon
    spiHw->dma_in_link.addr            = 0;
    spiHw->dma_in_link.start           = 1;

    spiHw->user1.usr_addr_bitlen       = 0;
    spiHw->user2.usr_command_bitlen    = 0;
    spiHw->user.usr_addr               = 0;
    spiHw->user.usr_command            = 0;
    if(waitCycle <= 0) {
        spiHw->user.usr_dummy              = 0;
        spiHw->user1.usr_dummy_cyclelen    = 0;
    } else {
        spiHw->user.usr_dummy              = 1;
        spiHw->user1.usr_dummy_cyclelen    = (uint8_t) (waitCycle-1);
    }

    spiHw->user.usr_mosi_highpart      = 0;
    spiHw->user2.usr_command_value     = 0;
    spiHw->addr                        = 0;
    spiHw->user.usr_mosi               = 1;        // Enable MOSI
    spiHw->user.usr_miso               = 0;

    spiHw->dma_out_link.addr           = (int)(lldescs) & 0xFFFFF;

	spiHw->mosi_dlen.usr_mosi_dbitlen  = 0;	
    spiHw->miso_dlen.usr_miso_dbitlen  = 0;

    // Set circular mode
    //      https://www.esp32.com/viewtopic.php?f=2&t=4011#p18107
    //      > yes, in SPI DMA mode, SPI will alway transmit and receive
    //      > data when you set the SPI_DMA_CONTINUE(BIT16) of SPI_DMA_CONF_REG.
    spiHw->dma_conf.dma_continue       = 1;

    spiHw->dma_conf.dma_tx_stop		= 1;	// Stop SPI DMA
    spiHw->ctrl2.val           		= 0;	// Reset timing
    spiHw->dma_conf.dma_tx_stop		= 0;	// Disable stop
    spiHw->dma_conf.dma_continue	= 1;	// Set contiguous mode
    spiHw->dma_out_link.start		= 1;	// Start SPI DMA transfer (1)
    // Start
    spiHw->cmd.usr					= 1;

    return ESP_OK;
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
    _row = 0;
    _data = data;
    _bytes_per_row = total_bytes_per_row;

    lldesc_s * lldescs = (lldesc_s*) heap_caps_calloc(1, rows * sizeof(lldesc_s), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    for(int i = 0; i < rows; i++) {
        lldesc_s * d = &lldescs[i];

        d->buf = &data[i * total_bytes_per_row];
        d->length = total_bytes_per_row;
        d->size = total_bytes_per_row;
        d->eof = 0;
        d->sosf = 0;
        d->owner = LLDESC_HW_OWNED;

        // make buffer circular
        if(i == (rows - 1)) {
            d->qe.stqe_next = &lldescs[0];
        } 
        else {
            d->qe.stqe_next = &lldescs[i + 1];
        }
    }

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
        // spi_device_interface_config_t dev_cfg = {
        //     .command_bits = 0,
        //     .address_bits = 0,
        //     .dummy_bits = 0,
        //     .mode = 3,
        //     .duty_cycle_pos = 0,
        //     .cs_ena_pretrans = 0,
        //     .cs_ena_posttrans = 0,
        //     .clock_speed_hz = PIXEL_CLOCK_HZ,
        //     .input_delay_ns = 0,
        //     .spics_io_num = LATCH_PIN,
        //     .flags = SPI_DEVICE_NO_DUMMY | SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_BIT_LSBFIRST | SPI_DEVICE_POSITIVE_CS,
        //     .queue_size = 128,
        //     .pre_cb = nullptr,
        //     .post_cb = nullptr
        // };

        myspi_prepare_circular_buffer(
            spi,
            lldescs,
            PIXEL_CLOCK_HZ,
            SIN1_PIN,
            SIN2_PIN,
            SIN3_PIN,
            SACRIFICIAL_UNUSE_PIN,
            LATCH_PIN
        );
    }
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
    }

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