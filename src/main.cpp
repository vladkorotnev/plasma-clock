#include <Arduino.h>
#include <plasma/iface.h>
#include <plasma/framebuffer.h>
#include <fonts.h>
#include <console.h>
#include "hw_config.h"
#include <AM232X.h>
#include <sensor/sensors.h>
#include <network/netmgr.h>

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
static SensorPool * sensors;

void beep(int f, int t) {
    ledcWriteTone(0, f);
    delay(t);
    ledcWrite(0, 0);
}

void setup() {
    ledcSetup(0, 2000, 8);
    ledcAttachPin(HWCONF_BEEPER_GPIO, 0);

    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    plasma.set_show(true);
    plasma.set_bright(false);

    fb = new PlasmaDisplayFramebuffer(&plasma);
    con = new Console(&keyrus0808_font, fb);
    con->set_cursor(true);
    
    con->print("SAS-DOS v1.0");
    delay(1000);
 
    beep(2000, 125);
    beep(1000, 125);

    con->clear();
    con->set_font(&sg8bit_font);
    con->set_cursor(true);

    con->print("WiFi init.");
    NetworkManager::startup();
    while(!NetworkManager::is_up()) {
        delay(1000);
        con->write('.');
    }

    con->clear();
    con->print("Network:");
    con->print(NetworkManager::network_name());
    delay(2000);
    con->print(NetworkManager::current_ip());
    delay(2000);

    sensors = new SensorPool();

    sensors->add(SENSOR_ID_AMBIENT_LIGHT, new AmbientLightSensor(HWCONF_LIGHTSENSE_GPIO), pdMS_TO_TICKS(1000));
    con->print("L sensor OK");

    sensors->add(SENSOR_ID_MOTION, new MotionSensor(HWCONF_MOTION_GPIO), pdMS_TO_TICKS(1000));
    con->print("M sensor OK");

    Wire.begin(HWCONF_I2C_SDA_GPIO, HWCONF_I2C_SCL_GPIO);
    AM2322* tempSens = new AM2322(&Wire);

    if(!sensors->add(SENSOR_ID_AMBIENT_TEMPERATURE, new Am2322TemperatureSensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("T sens err");
        beep(500, 125);
    } else {
        con->print("T sensor OK");
    }
    delay(3000);
    if(!sensors->add(SENSOR_ID_AMBIENT_HUMIDITY, new Am2322HumiditySensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("H sens err");
        beep(500, 125);
    } else {
        con->print("H sensor OK");
    }


   // vTaskDelete(NULL); // Get rid of setup() and loop() task
}

void loop() {
    static bool motn;
    bool motnNew;
    motnNew = (sensors->get_info(SENSOR_ID_MOTION)->last_result > 0);
    if(motnNew != motn) {
        con->print("MOT: %s", motnNew ? "YES" : "no");
        // ledcWriteTone(0, motnNew ? 2000:1000);
        delay(125);
        ledcWrite(0, 0);
        motn = motnNew;
    }

    static int light;
    int lightNew = sensors->get_info(SENSOR_ID_AMBIENT_LIGHT)->last_result;

    if(abs(light - lightNew) > 500) {
        con->print("Light: %i", lightNew);
        light = lightNew;
        if(light > 2500) {
            plasma.set_bright(true);
        } else if (light < 1000) {
            plasma.set_bright(false);
        }
    }

    static long lastTempRead = millis();
    if(millis() - lastTempRead > 5000) {
        sensor_info_t * hum = sensors->get_info(SENSOR_ID_AMBIENT_HUMIDITY);
        sensor_info_t * temp = sensors->get_info(SENSOR_ID_AMBIENT_TEMPERATURE);

        if(hum != nullptr && temp != nullptr) {
            con->print("T%i H%i", temp->last_result, hum->last_result);
        }

        lastTempRead = millis();
    }
}
