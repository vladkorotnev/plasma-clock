#include <Arduino.h>
#include <device_config.h>

#if HAS(OUTPUT_MD_PLASMA)
#include <display/md_plasma.h>
#elif HAS(OUTPUT_WS0010)
#include <display/ws0010.h>
#endif

#include <graphics/framebuffer.h>
#include <fonts.h>
#include <console.h>
#include <AM232X.h>
#include <sensor/sensors.h>
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
#include <network/admin_panel.h>
#include <utils.h>
#include <state.h>
#include <idle.h>

static char LOG_TAG[] = "APL_MAIN";

static device_state_t current_state = STATE_BOOT;
const device_state_t startup_state = STATE_IDLE;

#if HAS(OUTPUT_MD_PLASMA)
static MorioDenkiPlasmaDriver display_driver(
    HWCONF_PLASMA_DATABUS_GPIOS,
    HWCONF_PLASMA_CLK_GPIO,
    HWCONF_PLASMA_RESET_GPIO,
    HWCONF_PLASMA_BRIGHT_GPIO,
    HWCONF_PLASMA_SHOW_GPIO,
    HWCONF_PLASMA_HV_EN_GPIO
);
#elif HAS(OUTPUT_WS0010)
static Ws0010OledDriver display_driver(
    HWCONF_WS0010_DATABUS_GPIOS,
    HWCONF_WS0010_RS_GPIO,
    HWCONF_WS0010_EN_GPIO
);
#else
#error Output type not selected
#endif

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

    switch(to) {
        case STATE_IDLE:
            app_idle_prepare(sensors, beepola);
            break;
        case STATE_OTAFVU:
        default:
            break;
    }
    current_state = to;
}

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
    
    con->print(PRODUCT_NAME " v" PRODUCT_VERSION "\n");
    delay(500);
 
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
    con->print(NetworkManager::network_name());
    delay(2000);
    con->print(NetworkManager::current_ip().c_str());
    delay(2000);

    ota = new OTAFVUManager(con, bs);

    sensors = new SensorPool();

#if HAS(LIGHT_SENSOR)
    sensors->add(SENSOR_ID_AMBIENT_LIGHT, new AmbientLightSensor(HWCONF_LIGHTSENSE_GPIO), pdMS_TO_TICKS(250));
    con->print("L sensor OK");
#endif

#if HAS(MOTION_SENSOR)
    sensors->add(SENSOR_ID_MOTION, new MotionSensor(HWCONF_MOTION_GPIO), pdMS_TO_TICKS(1000));
    con->print("M sensor OK");
#endif

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
}

void drawing() {
    switch(current_state) {
        case STATE_IDLE:
            app_idle_draw(graph);
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
