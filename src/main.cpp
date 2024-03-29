#include <Arduino.h>
#include <plasma/iface.h>
#include <plasma/framebuffer.h>
#include <fonts.h>
#include <console.h>
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
static Console * con;

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    plasma.set_show(true);
    plasma.set_bright(false);

    fb = new PlasmaDisplayFramebuffer(&plasma);
    con = new Console(&keyrus0816_font, fb);
    con->set_cursor(true);
    con->print("BOOT");

    delay(1000);

    con->set_font(&keyrus0808_font);
    con->write('\n');
    delay(500);

   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}


void loop() {
    const char * sas = "Wake up, Neo.";
    const char * sus = "You obosralsya.";
    for(int i = 0; i < strlen(sas); i++) {
        con->write(sas[i]);
        delay(100);
    }
    delay(1000);
    con->write('\n');
     for(int i = 0; i < strlen(sus); i++) {
        con->write(sus[i]);
        delay(100);
    }
    delay(1000);
}
