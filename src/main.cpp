#include <Arduino.h>
#include <plasma/iface.h>
#include "hw_config.h"

static char LOG_TAG[] = "APL_MAIN";

PlasmaDisplayIface plasma(
    HWCONF_PLASMA_DATABUS_GPIOS,
    HWCONF_PLASMA_CLK_GPIO,
    HWCONF_PLASMA_RESET_GPIO,
    HWCONF_PLASMA_RTZ_GPIO,
    HWCONF_PLASMA_BRIGHT_GPIO,
    HWCONF_PLASMA_SHOW_GPIO
);

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_show(true);
    plasma.set_bright(false);

    for(int i = 0; i < 202; i++) {
        plasma.write_stride(0x55);
    }

   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}

uint16_t now = 1;
bool dir = true;

void loop() {
    if(now == 0x8000) dir = false;
    else if(now == 1) dir = true;

    plasma.write_column(now);

    if(dir) now <<= 1;
    else now >>= 1;

    delay(10);
}
