#include "idle.h"
#include <stdint.h>
#include <service/time.h>
#include <service/owm/weather.h>
#include <views/simple_clock.h>
#include <views/rain_ovl.h>
#include <views/indoor_view.h>
#include <views/framework.h>

static char LOG_TAG[] = "APL_IDLE";

typedef enum MainViewId: uint16_t {
    VIEW_CLOCK = 0,
    VIEW_INDOOR_WEATHER,

    VIEW_MAX
} MainViewId_t;

static int screen_times_ms[VIEW_MAX] = {
    30000, // VIEW_CLOCK
    10000, // VIEW_INDOOR_WEATHER
};

static bool did_prepare = false;

static Beeper * beepola;
static SensorPool * sensors;

static SimpleClock * clockView;
static RainOverlay * rain;
static IndoorView * indoorView;
static ViewMultiplexor * slideShow;

static MainViewId_t curScreen = VIEW_CLOCK;
static TickType_t lastScreenSwitch = 0;

static bool have_weather;
static current_weather_t weather;

static bool tick_tock = false;

void sound_tick_tock() {
    tk_time_of_day_t now = get_current_time_precise();
    if(now.millisecond >= 250 && !tick_tock) {
        beepola->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
        tick_tock = true;
    } else if (now.millisecond < 250 && tick_tock) {
        tick_tock = false;
    }
}

void app_idle_prepare(SensorPool* s, Beeper* b) {
    if(did_prepare) return;

    did_prepare = true;
    beepola = b;
    sensors = s;
    clockView = new SimpleClock();
    indoorView = new IndoorView(sensors);
    rain = new RainOverlay(101, 16);

    slideShow = new ViewMultiplexor();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(clockView);
    rainyClock->add_layer(rain);

    slideShow->add_view(rainyClock, VIEW_CLOCK);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);

    lastScreenSwitch = xTaskGetTickCount();

    rain->prepare();
    rain->set_intensity(20);
}

void change_screen_if_needed() {
    TickType_t now = xTaskGetTickCount();
    if(now - lastScreenSwitch >= pdMS_TO_TICKS(screen_times_ms[curScreen])) {
        curScreen = (MainViewId_t) (((uint16_t) curScreen) + 1);
        if(curScreen == VIEW_MAX) curScreen = VIEW_CLOCK;
        slideShow->switch_to(curScreen);
        lastScreenSwitch = now;
    }
}

void app_idle_draw(FantaManipulator* graph) {
    graph->clear();
    slideShow->render(graph);
}

void app_idle_process() {
    slideShow->step();
    if(weather_get_current(&weather)) {
        have_weather = true;
    }

    change_screen_if_needed();
    sound_tick_tock();
}