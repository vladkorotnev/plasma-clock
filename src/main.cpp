#include <Arduino.h>
#include <plasma/iface.h>
#include <plasma/framebuffer.h>
#include "hw_config.h"

static char LOG_TAG[] = "APL_MAIN";

static PlasmaDisplayIface plasma(
    HWCONF_PLASMA_DATABUS_GPIOS,
    HWCONF_PLASMA_CLK_GPIO,
    HWCONF_PLASMA_RESET_GPIO,
    HWCONF_PLASMA_BRIGHT_GPIO,
    HWCONF_PLASMA_SHOW_GPIO,
    HWCONF_PLASMA_HV_EN_GPIO
);

static PlasmaDisplayFramebuffer * fb;

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    plasma.set_show(true);
    plasma.set_bright(false);

    fb = new PlasmaDisplayFramebuffer(&plasma);

    ESP_LOGI(LOG_TAG, "Plot");

    for(int i = 0; i < 101; i++) {
        fb->plot_pixel(i, i % 16, true);
    }

    delay(2000);

   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}

uint16_t now = 1;
bool dir = true;

void loop() {
    fb->plot_pixel(random(0, 101), random(0, 16), true);
    fb->plot_pixel(random(0, 101), random(0, 16), false);

    delay(10);
}
