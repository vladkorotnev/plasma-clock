#include "idle.h"
#include <stdint.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/time.h>
#include <service/owm/weather.h>
#include <service/prefs.h>
#include <views/simple_clock.h>
#include <views/rain_ovl.h>
#include <views/thunder_ovl.h>
#include <views/indoor_view.h>
#include <views/framework.h>
#include <views/current_weather.h>

static char LOG_TAG[] = "APL_IDLE";

typedef enum MainViewId: uint16_t {
    VIEW_CLOCK = 0,
    VIEW_INDOOR_WEATHER,
    VIEW_OUTDOOR_WEATHER,

    VIEW_MAX
} MainViewId_t;

static int screen_times_ms[VIEW_MAX] = {
    30000, // VIEW_CLOCK
    10000, // VIEW_INDOOR_WEATHER
    25000, // VIEW_OUTDOOR_WEATHER
};

static bool did_prepare = false;

static Beeper * beepola;
static BeepSequencer * sequencer;
static SensorPool * sensors;

static Renderable * mainView;

static SimpleClock * clockView;

static RainOverlay * rain;
static ThunderOverlay * thunder;

static IndoorView * indoorView;
static CurrentWeatherView * weatherView;

static ViewMultiplexor * slideShow;

static MainViewId_t curScreen = VIEW_CLOCK;
static TickType_t lastScreenSwitch = 0;

static current_weather_t weather = { 0 };

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
    int first_hour = prefs_get_int(PREFS_KEY_HOURLY_CHIME_START_HOUR) ;
    if(now.hour != last_chimed_hour 
    && now.hour >= first_hour
    && now.hour <= prefs_get_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR)
    ) {
        last_chimed_hour = now.hour;
        if(!hourly_chime_on) return;
        int melody_no = (now.hour == first_hour) ? prefs_get_int(PREFS_KEY_FIRST_CHIME_MELODY) : prefs_get_int(PREFS_KEY_HOURLY_CHIME_MELODY);
        if(melody_no == all_chime_count) {
            melody_no = esp_random() % all_chime_count;
        }
        melody_sequence_t melody = all_chime_list[melody_no];

        sequencer->play_sequence(melody, CHANNEL_CHIME, 0);
    }
}

void weather_overlay_update() {
    ESP_LOGI(LOG_TAG, "Weather overlay: %i", weather.conditions);

    switch(weather.conditions) {
        case RAIN:
            rain->set_windspeed(0, false);
            rain->set_intensity(10);
            rain->set_speed_divisor(1);
            thunder->set_active(false);
            break;

        case DRIZZLE:
            rain->set_windspeed(0, false);
            rain->set_gravity(1, true);
            rain->set_intensity(5);
            rain->set_speed_divisor(2);
            thunder->set_active(false);
            break;

        case THUNDERSTORM:
            rain->set_windspeed(-1, false);
            rain->set_gravity(1, false);
            rain->set_intensity(13);
            rain->set_speed_divisor(2);
            thunder->set_active(true);
            break;

        case SNOW:
            rain->set_windspeed(-1, true);
            rain->set_gravity(1, false);
            rain->set_intensity(6);
            rain->set_speed_divisor(3);
            thunder->set_active(false);
            break;

        case CLEAR:
        case CLOUDS:
        case SCATTERED_CLOUDS:
        case BROKEN_CLOUDS:
        case OVERCAST:
        case UNKNOWN:
        default:
            // No rain, no snow, nothing
            rain->set_intensity(0);
            thunder->set_active(false);
            break;
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
    screen_times_ms[VIEW_OUTDOOR_WEATHER] = std::max(prefs_get_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS) * 1000, 1000);

    clockView = new SimpleClock();
    indoorView = new IndoorView(sensors);
    rain = new RainOverlay(101, 16);
    thunder = new ThunderOverlay(101, 16);
    weatherView = new CurrentWeatherView();

    // thunder hurts readability on other views, so keep it on clock only
    ViewCompositor * thunderClock = new ViewCompositor();
    thunderClock->add_layer(clockView);
    thunderClock->add_layer(thunder);

    slideShow = new ViewMultiplexor();
    slideShow->add_view(thunderClock, VIEW_CLOCK);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);
    slideShow->add_view(weatherView, VIEW_OUTDOOR_WEATHER);

    lastScreenSwitch = xTaskGetTickCount();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(slideShow);
    rainyClock->add_layer(rain);
    mainView = rainyClock;

    mainView->prepare();
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

    current_weather_t w;
    if(weather_get_current(&w) && w.last_updated != weather.last_updated) {
        memcpy(&weather, &w, sizeof(current_weather_t));
        weather_overlay_update();
    }

    change_screen_if_needed();

    hourly_chime();

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);
    hourly_chime_on = prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON);
}