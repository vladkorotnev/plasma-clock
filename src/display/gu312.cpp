#include <device_config.h>

// This is a result of reverse engineering a bus display that uses a half meter long GU192x16-321. 
// Mostly similar to the old noritake protocol but there are some undocumented states.
// The pinout is also undocumented so here goes
//    1        2 
//     +______+
//  D7>| .  . | GND
//  D6 | .  . | GND
//  D5 | .  . | GND
//  D4 | .  . | GND
//  D3 | .  . | GND
//  D2 | .  . | GND
//  D1 | .  . | GND
//  D0 | .  . | GND
// ~WR | .  .   GND
//  A0 | .  . | GND
// ~RD | .  . | GND
//  A1 | .  . | ???
// FEP | .  . | ~BL         <-- note: FEP is Vsync out FROM the display!. ~BL=0 turns display off.
// GND | .  . | GND
//~RST | .  . | GND
//  A2 | .  . | GND
// ~CS | .  . | GND
//     +------+
//    33     34

#if HAS(OUTPUT_GU312)
#include "display/gu312.h"
#include <esp32-hal-log.h>
#include <esp32-hal-gpio.h>
#include <esp_err.h>

static char LOG_TAG[] = "Gu312";
static portMUX_TYPE _spinlock = portMUX_INITIALIZER_UNLOCKED;

NoritakeGu312Driver::NoritakeGu312Driver(
    const gpio_num_t databus[8],
    const gpio_num_t cd,
    const gpio_num_t wr,
    const gpio_num_t blank,
    const gpio_num_t vsync
) {
    for(int i = 0; i < 8; i++) {
        databus_gpios[i] = databus[i];
    }
    cd_gpio = cd;
    wr_gpio = wr;
    blank_gpio = blank;
    fep_gpio = vsync;
}

void NoritakeGu312Driver::initialize() {
    ESP_LOGI(LOG_TAG, "Initializing Noritake Itron GU312 with data bus: %i %i %i %i %i %i %i %i, cd=%i, wr=%i, bl=%i", databus_gpios[0], databus_gpios[1], databus_gpios[2], databus_gpios[3], databus_gpios[4], databus_gpios[5], databus_gpios[6], databus_gpios[7], cd_gpio, wr_gpio, blank_gpio);

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_ENABLE,
    };

    for(int i = 0; i < sizeof(databus_gpios) / sizeof(databus_gpios[0]); i++) {
        io_conf.pin_bit_mask |= 1ULL << databus_gpios[i];
    }

    io_conf.pin_bit_mask |= 1ULL << cd_gpio;
    io_conf.pin_bit_mask |= 1ULL << wr_gpio;
    io_conf.pin_bit_mask |= 1ULL << blank_gpio;

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(blank_gpio, 1);
    gpio_set_level(wr_gpio, 1);
    gpio_set_level(cd_gpio, 0);

    if(fep_gpio != GPIO_NUM_NC) {
        gpio_config_t fep_conf = {
            .pin_bit_mask = 1ULL << fep_gpio,
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
            .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&io_conf));
    }

    delay(500);
}

void NoritakeGu312Driver::set_databus(uint8_t data) {
    uint8_t local_sts = data;
    for(int i = 0; i < 8; i++) {
        uint8_t cur_state = (local_sts & 1);
        gpio_set_level(databus_gpios[i], cur_state);
        local_sts >>= 1;
    }
}

void NoritakeGu312Driver::pulse_clock() {
    gpio_set_level(wr_gpio, 0);
    delayMicroseconds(2);
    gpio_set_level(wr_gpio, 1);
    delayMicroseconds(2);
}

void NoritakeGu312Driver::set_is_command(bool rs) {
    // H: COMMAND, L: Data
    gpio_set_level(cd_gpio, rs ? 1 : 0);
    delayMicroseconds(3);
}

void NoritakeGu312Driver::send_command(uint8_t cmd) {
    set_is_command(true);
    set_databus(cmd);
    pulse_clock();
    set_is_command(false);
}

void NoritakeGu312Driver::reset() {
    ESP_LOGI(LOG_TAG, "Resetting");
     taskENTER_CRITICAL(&_spinlock);

    send_command(0b11000); // brightness max
    send_command(0b00010); // SCR2 on
    send_command(0b00111); // SCR2 graphic
    send_command(0b00100); // cursor auto increment

    send_command(0b01010); // set read pos SCR#1 Low
        set_databus(0x80); pulse_clock();
    send_command(0b01011); // set read pos SCR#1 High
        set_databus(0x1); pulse_clock();

    send_command(0b01100); // set read pos SCR#2 Low
        set_databus(0x0); pulse_clock();
    send_command(0b01101); // set read pos SCR#2 High
        set_databus(0xc); pulse_clock();

    // put all 0 in SCR1
    send_command(0b01110); // set write pos Low
        set_databus(0x0); pulse_clock();
    send_command(0b01111); // set write pos High
        set_databus(0xc); pulse_clock();
    send_command(0b01000); // WRITE mode
   
    for(int i = 0; i < 192 * 8; i++) {
        set_databus(0xAA);
        pulse_clock();
    }

    ESP_LOGI(LOG_TAG, "Reset complete");

     taskEXIT_CRITICAL(&_spinlock);
    //   while(true) vTaskDelay(portMAX_DELAY);
}

void NoritakeGu312Driver::set_power(bool power) {
    gpio_set_level(blank_gpio, power ? 1 : 0);
}

void NoritakeGu312Driver::clear() {
    ESP_LOGW(LOG_TAG, "Not supported!");
}

inline uint8_t flipByte(uint8_t c){
  char r=0;
  for(uint8_t i = 0; i < 8; i++){
    r <<= 1;
    r |= c & 1;
    c >>= 1;
  }
  return r;
}

void NoritakeGu312Driver::write_fanta(const uint8_t * strides, size_t count) {
    taskENTER_CRITICAL(&_spinlock);
    if(fep_gpio != GPIO_NUM_NC) {
        // wait for frame pulse
        while(!gpio_get_level(fep_gpio));
        while(gpio_get_level(fep_gpio));
    }

    send_command(0b01110); // set write pos Low
        set_databus(0); pulse_clock();
    send_command(0b01111); // set write pos High
        set_databus(0xc); pulse_clock();
    send_command(0b01000); // WRITE mode
    for(int i = 0; i < count; i++) {
        set_databus(flipByte(strides[i]));
        pulse_clock();
        if(i % 2 != 0) {
            set_databus(0);
            for(int i = 0; i < 6; i++) pulse_clock();
        }
    }
    send_command(0b00010); 
    taskEXIT_CRITICAL(&_spinlock);
}

void NoritakeGu312Driver::set_bright(bool bright) {
    taskENTER_CRITICAL(&_spinlock);
    send_command(bright ? 0b11001 : 0b11110);
    // undocumented: can go from 11000 (100%) to as low as 11111 (undocumented)
    taskEXIT_CRITICAL(&_spinlock);
}

#endif