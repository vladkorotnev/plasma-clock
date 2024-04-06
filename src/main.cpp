#include <Arduino.h>
#include <plasma/iface.h>
#include <plasma/framebuffer.h>
#include <fonts.h>
#include <console.h>
#include "hw_config.h"
#include <AM232X.h>
#include <sensor/sensors.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <network/netmgr.h>
#include <network/otafvu.h>
#include <service/power_management.h>
#include <service/time.h>
#include <views/simple_clock.h>
#include <utils.h>

static char LOG_TAG[] = "APL_MAIN";

static bool drawing_suspended = false;
void set_suspend_all_drawing(bool sus) {
    drawing_suspended = sus;
}

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
static OTAFVUManager * ota;
static Beeper * beepola;
static BeepSequencer * bs;
static SimpleClock * clockView;

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    delay(500);
    plasma.set_show(true);
    plasma.set_bright(true);

    fb = new PlasmaDisplayFramebuffer(&plasma);
    con = new Console(&keyrus0808_font, fb);
    con->set_cursor(true);
    
    // Plasma Information System DOS
    con->print("PIS-DOS v1.0\n");
    delay(1000);
 
    beepola = new Beeper(HWCONF_BEEPER_GPIO, HWCONF_BEEPER_PWM_CHANNEL);
    bs = new BeepSequencer(beepola);
    bs->play_sequence(pc98_pipo, CHANNEL_SYSTEM, 0);

    con->clear();
    con->set_font(&sg8bit_font);
    con->set_cursor(true);

    con->print("WiFi init");
    NetworkManager::startup();
    while(!NetworkManager::is_up()) {
        delay(1000);
        con->write('.');
    }

    con->clear();
    con->print("Network:");
    con->print(NetworkManager::network_name());
    delay(2000);
    con->print(NetworkManager::current_ip().c_str());
    delay(2000);

    ota = new OTAFVUManager(con, bs);

    sensors = new SensorPool();

    sensors->add(SENSOR_ID_AMBIENT_LIGHT, new AmbientLightSensor(HWCONF_LIGHTSENSE_GPIO), pdMS_TO_TICKS(250));
    con->print("L sensor OK");

    sensors->add(SENSOR_ID_MOTION, new MotionSensor(HWCONF_MOTION_GPIO), pdMS_TO_TICKS(1000));
    con->print("M sensor OK");

    Wire.begin(HWCONF_I2C_SDA_GPIO, HWCONF_I2C_SCL_GPIO);
    AM2322* tempSens = new AM2322(&Wire);

    if(!sensors->add(SENSOR_ID_AMBIENT_TEMPERATURE, new Am2322TemperatureSensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("T sens err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    } else {
        con->print("T sensor OK");
    }
    delay(3000);
    if(!sensors->add(SENSOR_ID_AMBIENT_HUMIDITY, new Am2322HumiditySensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("H sens err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    } else {
        con->print("H sensor OK");
    }

    clockView = new SimpleClock(fb->manipulate(), beepola);

    // Finish all preparations, clear the screen and disable console

    con->set_active(false);
    fb->clear();
    timekeeping_begin();
    power_mgmt_start(sensors, &plasma, beepola);
}

void loop() {
    if(drawing_suspended) {
        return;
    }

    clockView->render();
}
