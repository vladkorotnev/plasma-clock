#include "idle.h"
#include <stdint.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/time.h>
#include <service/owm/weather.h>
#include <service/prefs.h>
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
static BeepSequencer * sequencer;
static SensorPool * sensors;

static Renderable * mainView;

static SimpleClock * clockView;
static RainOverlay * rain;
static IndoorView * indoorView;
static ViewMultiplexor * slideShow;

static MainViewId_t curScreen = VIEW_CLOCK;
static TickType_t lastScreenSwitch = 0;

static bool have_weather;
static current_weather_t weather;

static bool tick_tock_enable = false;
static bool tick_tock = false;

static bool hourly_chime_on = false;
static int last_chimed_hour = 0;

void sound_tick_tock() {
    tk_time_of_day_t now = get_current_time_precise();
    if(now.millisecond >= 250 && !tick_tock) {
        beepola->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
        tick_tock = true;
    } else if (now.millisecond < 250 && tick_tock) {
        tick_tock = false;
    }
}

void hourly_chime() {
    static tk_time_of_day now;
    now = get_current_time_coarse();
    if(now.hour != last_chimed_hour 
    && now.hour >= prefs_get_int(PREFS_KEY_HOURLY_CHIME_START_HOUR) 
    && now.hour <= prefs_get_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR)
    ) {
        last_chimed_hour = now.hour;
        if(!hourly_chime_on) return;
        int melody_no = prefs_get_int(PREFS_KEY_HOURLY_CHIME_MELODY);
        if(melody_no == all_chime_count) {
            melody_no = esp_random() % all_chime_count;
        }
        melody_sequence_t melody = all_chime_list[melody_no];

        sequencer->play_sequence(melody, CHANNEL_CHIME, 0);
    }
}

void app_idle_prepare(SensorPool* s, Beeper* b) {
    if(did_prepare) return;

    did_prepare = true;
    beepola = b;
    sequencer = new BeepSequencer(beepola);
    sensors = s;

    tk_time_of_day now = get_current_time_coarse();
    last_chimed_hour = now.hour;

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);
    hourly_chime_on = prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON);

    screen_times_ms[VIEW_CLOCK] = std::max(prefs_get_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS) * 1000, 1000);
    screen_times_ms[VIEW_INDOOR_WEATHER] = std::max(prefs_get_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS) * 1000, 1000);

    clockView = new SimpleClock();
    indoorView = new IndoorView(sensors);
    rain = new RainOverlay(101, 16);

    slideShow = new ViewMultiplexor();
    slideShow->add_view(clockView, VIEW_CLOCK);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);

    lastScreenSwitch = xTaskGetTickCount();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(slideShow);
    rainyClock->add_layer(rain);
    mainView = rainyClock;

    mainView->prepare();
    rain->set_intensity(20);
}

void change_screen_if_needed() {
    TickType_t now = xTaskGetTickCount();
    if(now - lastScreenSwitch >= pdMS_TO_TICKS(screen_times_ms[curScreen])) {
        curScreen = (MainViewId_t) (((uint16_t) curScreen) + 1);
        if(curScreen == VIEW_MAX) curScreen = VIEW_CLOCK;
        slideShow->switch_to(curScreen, (transition_type_t) prefs_get_int(PREFS_KEY_TRANSITION_TYPE));
        lastScreenSwitch = now;
    }
}

void app_idle_draw(FantaManipulator* graph) {
    graph->clear();
    mainView->render(graph);
}

void app_idle_process() {
    if(tick_tock_enable) {
        sound_tick_tock();
    }

    mainView->step();
    if(weather_get_current(&weather)) {
        have_weather = true;
    }

    change_screen_if_needed();

    hourly_chime();

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);
    hourly_chime_on = prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON);
}