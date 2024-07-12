#include <Arduino.h>
#include <device_config.h>
#include <display/display.h>
#include <graphics/framebuffer.h>
#include <fonts.h>
#include <console.h>
#include <sensor/sensors.h>
#include <input/touch_plane.h>
#include <input/hid_sensor.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <network/netmgr.h>
#include <network/otafvu.h>
#include <service/power_management.h>
#include <service/owm/weather.h>
#include <service/time.h>
#include <service/prefs.h>
#include <service/wordnik.h>
#include <service/foo_client.h>
#include <service/alarm.h>
#include <network/admin_panel.h>
#include <utils.h>
#include <state.h>
#include <app/idle.h>
#include <app/alarming.h>
#include <sensor/switchbot/meter.h>

static char LOG_TAG[] = "APL_MAIN";

static device_state_t current_state = STATE_BOOT;
const device_state_t startup_state = STATE_IDLE;

static DisplayFramebuffer * fb;
static FantaManipulator * graph;
static Console * con;
static SensorPool * sensors;
static OTAFVUManager * ota;
static Beeper * beepola;
static BeepSequencer * bs;

static bool fps_counter = false;

void change_state(device_state_t to) {
    if(to == STATE_OTAFVU) {
        current_state = STATE_OTAFVU;
        return; // all other things handled in the FVU process
    }

    if(to == current_state) return;

    switch(to) {
        case STATE_IDLE:
            app_idle_prepare(sensors, beepola);
            break;
        case STATE_ALARMING:
            app_alarming_prepare(beepola);
            break;
        case STATE_OTAFVU:
        default:
            break;
    }
    current_state = to;
}

void bringup_light_sensor() {
#if HAS(LIGHT_SENSOR)
    sensors->add(SENSOR_ID_AMBIENT_LIGHT, new AmbientLightSensor(HWCONF_LIGHTSENSE_GPIO), pdMS_TO_TICKS(250));
    con->print("L sensor OK");
#endif
}

void bringup_motion_sensor() {
#if HAS(MOTION_SENSOR)
    sensors->add(SENSOR_ID_MOTION, new MotionSensor(HWCONF_MOTION_GPIO), pdMS_TO_TICKS(1000));
    con->print("M sensor OK");
#endif
}

void bringup_temp_sensor() {
#if HAS(TEMP_SENSOR)
    Wire.begin(HWCONF_I2C_SDA_GPIO, HWCONF_I2C_SCL_GPIO);
    AM2322* tempSens = new AM2322(&Wire);

    if(!sensors->add(SENSOR_ID_AMBIENT_TEMPERATURE, new Am2322TemperatureSensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("T sens err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    } else {
        con->print("T sensor OK");
    }
    delay(1000);
    if(!sensors->add(SENSOR_ID_AMBIENT_HUMIDITY, new Am2322HumiditySensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("H sens err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    } else {
        con->print("H sensor OK");
    }
#endif
}

void bringup_switchbot_sensor() {
#if HAS(SWITCHBOT_METER_INTEGRATION)
    if(prefs_get_bool(PREFS_KEY_SWITCHBOT_METER_ENABLE)) {
        String woMeterMac = prefs_get_string(PREFS_KEY_SWITCHBOT_METER_MAC);
        size_t tmpLen = woMeterMac.length();
        if(tmpLen > 0) {
            SwitchbotMeterApi *wometer = new SwitchbotMeterApi(woMeterMac.c_str());

            SwitchbotMeterHumidity *remoteHum = new SwitchbotMeterHumidity(wometer);
            SwitchbotMeterTemperature *remoteTemp = new SwitchbotMeterTemperature(wometer);

            if(!sensors->add(SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY, remoteHum, pdMS_TO_TICKS(30000))) {
                con->print("WoH err");
            } else {
                con->print("WoH ok");
                if(prefs_get_bool(PREFS_KEY_SWITCHBOT_EMULATES_LOCAL) && !sensors->exists(SENSOR_ID_AMBIENT_HUMIDITY)) {
                    sensors->short_circuit(SENSOR_ID_AMBIENT_HUMIDITY, SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY);
                }
            }

            if(!sensors->add(SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE, remoteTemp, pdMS_TO_TICKS(30000))) {
                con->print("WoT err");
            } else {
                con->print("WoT ok");
                if(prefs_get_bool(PREFS_KEY_SWITCHBOT_EMULATES_LOCAL) && !sensors->exists(SENSOR_ID_AMBIENT_TEMPERATURE)) {
                    sensors->short_circuit(SENSOR_ID_AMBIENT_TEMPERATURE, SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE);
                }
            }
        }
    }
#endif
}

void bringup_touch() {
#if HAS(TOUCH_PLANE)
    con->print("Touch init");
    if(touchplane_start() != ESP_OK) {
        con->print("TP init err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    }
#endif
}

extern void touchplane_debug(Console *);

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    display_driver.reset();

    display_driver.set_power(true);
    delay(125);
    display_driver.set_show(true);
#if HAS(VARYING_BRIGHTNESS)
    display_driver.set_bright(true);
#endif

    fb = new DisplayFramebuffer(&display_driver);
    con = new Console(&keyrus0808_font, fb);
    con->set_cursor(true);
    con->print("");
    
    con->print(PRODUCT_NAME " v" PRODUCT_VERSION);
    delay(500);
    beepola = new Beeper(HWCONF_BEEPER_GPIO, HWCONF_BEEPER_PWM_CHANNEL);
    bs = new BeepSequencer(beepola);
    bs->play_sequence(pc98_pipo, CHANNEL_SYSTEM, SEQUENCER_NO_REPEAT);

    con->clear();
    con->set_font(&keyrus0808_font);
    con->set_cursor(true);

    // bringup_touch();
    // touchplane_debug(con);
    // vTaskDelay(portMAX_DELAY);
    // return;

    con->print("WiFi init");
    NetworkManager::startup();
    while(!NetworkManager::is_up()) {
        delay(1000);
        con->write('.');
    }

    con->clear();
    con->print(NetworkManager::network_name());
    con->print("%i dBm", NetworkManager::rssi());
    delay(2000);
    con->print(NetworkManager::current_ip().c_str());
    delay(2000);

    ota = new OTAFVUManager(con, bs);

    sensors = new SensorPool();

    sensors->add(VIRTSENSOR_ID_WIRELESS_RSSI, new RssiSensor(), pdMS_TO_TICKS(500));
    sensors->add(VIRTSENSOR_ID_HID_STARTLED, new HidActivitySensor(), pdMS_TO_TICKS(250));
    bringup_light_sensor();
    bringup_motion_sensor();
    bringup_temp_sensor();
    bringup_switchbot_sensor();
    bringup_touch();

    graph = fb->manipulate();

    timekeeping_begin();
    weather_start();
    wotd_start();
    foo_client_begin();
    power_mgmt_start(sensors, &display_driver, beepola);
    admin_panel_prepare(sensors, beepola);
    fps_counter = prefs_get_bool(PREFS_KEY_FPS_COUNTER);

    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);

    con->set_active(false);
    fb->clear();

    change_state(startup_state);
    alarm_init();
}

void drawing() {
    switch(current_state) {
        case STATE_IDLE:
            app_idle_draw(graph);
            break;

        case STATE_ALARMING:
            app_alarming_draw(graph);
            break;

        case STATE_OTAFVU:
            break;
        default:
            ESP_LOGE(LOG_TAG, "Unknown state %i", current_state);
            break;
    }

#if defined(PDFB_PERF_LOGS)
    if(fps_counter) {
        // FPS counter
        char buf[4];
        itoa(fb->get_fps(), buf, 10);
        fb->manipulate()->put_string(&fps_counter_font, buf, 0, 0);
    }
#endif
}

void processing() {
    switch(current_state) {
        case STATE_IDLE:
            app_idle_process();
            break;

        case STATE_ALARMING:
            app_alarming_process();
            break;

        case STATE_OTAFVU:
            break;
        default:
            ESP_LOGE(LOG_TAG, "Unknown state %i", current_state);
            break;
    }
}

void loop() {
    fb->wait_next_frame();
    if(graph->lock()) {
        drawing();
        graph->unlock();
    }
    processing();
}
