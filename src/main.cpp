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
#include <service/owm/weather.h>
#include <views/simple_clock.h>
#include <views/rain_ovl.h>
#include <views/indoor_view.h>
#include <views/multiplexor.h>
#include <views/compositor.h>
#include <utils.h>

static char LOG_TAG[] = "APL_MAIN";

typedef enum MainViewId: uint16_t {
    VIEW_CLOCK = 0,
    VIEW_INDOOR_WEATHER,

    VIEW_MAX
} MainViewId_t;

static MainViewId_t curScreen = VIEW_CLOCK;
static TickType_t lastScreenSwitch = 0;

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
static FantaManipulator * graph;
static Console * con;
static SensorPool * sensors;
static OTAFVUManager * ota;
static Beeper * beepola;
static BeepSequencer * bs;
static SimpleClock * clockView;
static RainOverlay * rain;
static IndoorView * indoorView;
static ViewMultiplexor * slideShow;

void setup() {
    // Set up serial for logs
    Serial.begin(115200);
    plasma.reset();

    plasma.set_power(true);
    delay(125);
    plasma.set_show(true);
    plasma.set_bright(true);

    fb = new PlasmaDisplayFramebuffer(&plasma);
    con = new Console(&keyrus0808_font, fb);
    con->set_cursor(true);
    
    // Plasma Information System DOS
    con->print("PIS-DOS v1.0\n");
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
    delay(1000);
    if(!sensors->add(SENSOR_ID_AMBIENT_HUMIDITY, new Am2322HumiditySensor(tempSens), pdMS_TO_TICKS(5000))) {
        con->print("H sens err");
        beepola->beep_blocking(CHANNEL_SYSTEM, 500, 125);
    } else {
        con->print("H sensor OK");
    }

    graph = fb->manipulate();

    timekeeping_begin();
    weather_start(WEATHER_API_KEY, pdMS_TO_TICKS(60 * 60000), WEATHER_LAT, WEATHER_LON);
    power_mgmt_start(sensors, &plasma, beepola);

    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);

    con->set_active(false);
    fb->clear();

    clockView = new SimpleClock(beepola);
    indoorView = new IndoorView(sensors);
    rain = new RainOverlay(graph->get_width(), graph->get_height());

    slideShow = new ViewMultiplexor();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(clockView);
    rainyClock->add_layer(rain);

    slideShow->add_view(rainyClock, VIEW_CLOCK);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);

    rain->prepare();
    rain->set_intensity(20);
}

static bool have_weather;
static current_weather_t weather;

void change_screen_if_needed() {
    TickType_t now = xTaskGetTickCount();
    if(now - lastScreenSwitch >= pdMS_TO_TICKS(5000)) {
        curScreen = (MainViewId_t) (((uint16_t) curScreen) + 1);
        if(curScreen == VIEW_MAX) curScreen = VIEW_CLOCK;
        slideShow->switch_to(curScreen);
        lastScreenSwitch = now;
    }
}

void draw_screen_locked() {
    // The framebuffer is now locked -- do not do any processing here!! Only drawing calls!
    graph->clear();
    slideShow->render(graph);

    if(sensors->get_info(SENSOR_ID_MOTION)->last_result > 0) {
        graph->plot_pixel(0, 0, true);
    }

    // ---- OVERLAYS 
    // if(have_weather && weather.conditions == RAIN) {
        // rain->render(graph);
    // }

#if defined(PDFB_PERF_LOGS) && defined(DRAW_FPS_COUNTER)
    // FPS counter
    char buf[4];
    itoa(fb->get_fps(), buf, 10);
    fb->manipulate()->put_string(&keyrus0808_font, buf, 0, 8);
#endif
}

void processing() {
    slideShow->step();
    if(weather_get_current(&weather)) {
        have_weather = true;
    }

    change_screen_if_needed();
}

void loop() {
    fb->wait_next_frame();
    if(!drawing_suspended && graph->lock()) {
        draw_screen_locked();
        graph->unlock();
    }
    processing();
}
