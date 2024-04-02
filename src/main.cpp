#include <Arduino.h>
#include <plasma/iface.h>
#include <plasma/framebuffer.h>
#include <fonts.h>
#include <console.h>
#include "hw_config.h"
#include <AM232X.h>

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

static AM2322 * tempSens;

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    plasma.set_show(true);
    plasma.set_bright(false);

    fb = new PlasmaDisplayFramebuffer(&plasma);
    con = new Console(&keyrus0816_font, fb);
    con->set_cursor(false);
    con->print("BOOT");

    delay(1000);

    ledcSetup(0, 2000, 8);
    ledcAttachPin(HWCONF_BEEPER_GPIO, 0);
    ledcWriteTone(0, 2000);
    delay(125);
    ledcWriteTone(0, 1000);
    delay(125);
    ledcWrite(0, 0);

    con->clear();
    con->set_font(&keyrus0808_font);
    con->set_cursor(true);

    pinMode(HWCONF_MOTION_GPIO, INPUT_PULLDOWN);
    pinMode(HWCONF_LIGHTSENSE_GPIO, ANALOG);

    Wire.begin(HWCONF_I2C_SDA_GPIO, HWCONF_I2C_SCL_GPIO);
    tempSens = new AM2322(&Wire);

    if(!tempSens->begin()) {
        ESP_LOGE(LOG_TAG, "Init temp sens error");
        con->print("T: Init Error\n");
    } else {
        tempSens->wakeUp();
        ESP_LOGI(LOG_TAG, "Temp sens wake up");
        con->print("T: OK\n");
    }

    delay(500);

    const char * sas = "Wake up, Neo";
    const char * sus = "You obosralsya";
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
    con->write('\n');

   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}

void loop() {
    static bool motn;
    bool motnNew;
    motnNew = digitalRead(HWCONF_MOTION_GPIO);
    if(motnNew != motn) {
        con->print("MOT: %s\n", motnNew ? "YES" : "no");
        motn = motnNew;
    }

    static int light;
    int lightNew = analogRead(HWCONF_LIGHTSENSE_GPIO);

    if(abs(light - lightNew) > 500) {
        con->print("Light: %i\n", lightNew);
        light = lightNew;
        if(light > 2500) {
            plasma.set_bright(true);
        } else if (light < 1000) {
            plasma.set_bright(false);
        }
    }

    static float oldTemp, oldHum;
    static long lastTempRead = millis();
    if(millis() - lastTempRead > 3000) {
        int status = tempSens->read();
        if(status != AM232X_OK) {
            ESP_LOGE(LOG_TAG, "Temp sens error %i\n", status);
            con->print("T: err %i\n", status);
        } else {
            float temp = tempSens->getTemperature();
            float hum = tempSens->getHumidity();
            if(temp != oldTemp || hum != oldHum) {
                con->print("T=%.2f, H=%.2f\n", temp, hum);
                oldTemp = temp;
                oldHum = hum;
            }
        }

        lastTempRead = millis();
    }
}
