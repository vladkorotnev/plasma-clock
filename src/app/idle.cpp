#include "app/idle.h"
#include <device_config.h>
#include <stdint.h>
#include <sound/sequencer.h>
#include <sound/melodies.h>
#include <service/time.h>
#include <service/owm/weather.h>
#include <service/prefs.h>
#include <service/foo_client.h>
#include <views/idle_screens/simple_clock.h>
#include <views/overlays/rain_ovl.h>
#include <views/overlays/thunder_ovl.h>
#if HAS(TEMP_SENSOR) || HAS(SWITCHBOT_METER_INTEGRATION)
#include <views/idle_screens/indoor_view.h>
#endif
#include <views/framework.h>
#include <views/idle_screens/current_weather.h>
#include <views/idle_screens/word_of_the_day.h>
#include <views/idle_screens/fb2k.h>
#include <views/overlays/signal_icon.h>
#include <views/overlays/touch_arrows_ovl.h>
#include <input/keys.h>
#include <state.h>

static char LOG_TAG[] = "APL_IDLE";

typedef enum MainViewId: uint16_t {
    VIEW_CLOCK = 0,
#if HAS(TEMP_SENSOR)
    VIEW_INDOOR_WEATHER,
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    VIEW_REMOTE_WEATHER,
#endif
    VIEW_OUTDOOR_WEATHER,
#if HAS(WORDNIK_API)
    VIEW_WORD_OF_THE_DAY,
#endif
    VIEW_FB2K,

    VIEW_MAX
} MainViewId_t;

static int screen_times_ms[VIEW_MAX] = {
    30000, // VIEW_CLOCK
#if HAS(TEMP_SENSOR)
    10000, // VIEW_INDOOR_WEATHER
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    10000, // VIEW_REMOTE_WEATHER,
#endif
    25000, // VIEW_OUTDOOR_WEATHER
#if HAS(WORDNIK_API)
    25000, // VIEW_WORD_OF_THE_DAY
#endif
    0, // VIEW_FB2K
};

int current_screen_time_ms = 0;

static bool did_prepare = false;

static Beeper * beepola;
static BeepSequencer * sequencer;
static SensorPool * sensors;

static Renderable * mainView;

static SimpleClock * clockView;

static RainOverlay * rain;
static ThunderOverlay * thunder;
static SignalStrengthIcon * signalIndicator;
static TouchArrowOverlay * touchArrows;

#if HAS(TEMP_SENSOR)
static IndoorView * indoorView;
#endif

#if HAS(SWITCHBOT_METER_INTEGRATION)
static WoSensorView * remoteWeatherView;
#endif

static CurrentWeatherView * weatherView;
#if HAS(WORDNIK_API)
static WordOfTheDayView * wotdView;
#endif
static Fb2kView *fb2kView;

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
    tk_time_of_day now = get_current_time_coarse();
    int first_hour = prefs_get_int(PREFS_KEY_HOURLY_CHIME_START_HOUR) ;
    if(now.hour != last_chimed_hour) {
        last_chimed_hour = now.hour;
        if(  hourly_chime_on
          && now.hour >= first_hour
          && now.hour <= prefs_get_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR)
        ) {
            int melody_no = (now.hour == first_hour) ? prefs_get_int(PREFS_KEY_FIRST_CHIME_MELODY) : prefs_get_int(PREFS_KEY_HOURLY_CHIME_MELODY);
            melody_sequence_t melody = melody_from_no(melody_no);

            sequencer->play_sequence(melody, CHANNEL_CHIME, SEQUENCER_NO_REPEAT);
        }
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

    screen_times_ms[VIEW_CLOCK] = prefs_get_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS) * 1000;
#if HAS(TEMP_SENSOR)
    screen_times_ms[VIEW_INDOOR_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS) * 1000;
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    screen_times_ms[VIEW_REMOTE_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS) * 1000;
#endif
    screen_times_ms[VIEW_OUTDOOR_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS) * 1000;
#if HAS(WORDNIK_API)
    screen_times_ms[VIEW_WORD_OF_THE_DAY] = prefs_get_int(PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS) * 1000;
#endif
    screen_times_ms[VIEW_FB2K] = prefs_get_int(PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS) * 1000;

    bool has_at_least_one_screen = false;
    for(int i = 0; i < VIEW_MAX; i++) {
        if(screen_times_ms[i] != 0) {
            has_at_least_one_screen = true;
            break;
        }
    }
    if(!has_at_least_one_screen) {
        screen_times_ms[VIEW_CLOCK] = 3600000;
    }

    current_screen_time_ms = screen_times_ms[VIEW_CLOCK];

    clockView = new SimpleClock();
    rain = new RainOverlay(101, 16);
    thunder = new ThunderOverlay(101, 16);
    signalIndicator = new SignalStrengthIcon(sensors);
    weatherView = new CurrentWeatherView();
    fb2kView = new Fb2kView();

    touchArrows = new TouchArrowOverlay();
    touchArrows->bottom = true;
    touchArrows->top = true;
    touchArrows->left = true;

    // thunder hurts readability on other views, so keep it on clock only
    ScreenCompositor * thunderClock = new ScreenCompositor(clockView);
    thunderClock->add_layer(thunder);

    slideShow = new ViewMultiplexor();
    slideShow->add_view(thunderClock, VIEW_CLOCK);
#if HAS(TEMP_SENSOR)
    indoorView = new IndoorView(sensors);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    remoteWeatherView = new WoSensorView(sensors);
    slideShow->add_view(remoteWeatherView, VIEW_REMOTE_WEATHER);
#endif
    slideShow->add_view(weatherView, VIEW_OUTDOOR_WEATHER);
#if HAS(WORDNIK_API)
    wotdView = new WordOfTheDayView();
    slideShow->add_view(wotdView, VIEW_WORD_OF_THE_DAY);
#endif
    slideShow->add_view(fb2kView, VIEW_FB2K);

    lastScreenSwitch = xTaskGetTickCount();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(slideShow);
    rainyClock->add_layer(signalIndicator);
    rainyClock->add_layer(rain);
    rainyClock->add_layer(touchArrows);
    mainView = rainyClock;

    mainView->prepare();
}

void update_screen_specific_time() {
    current_screen_time_ms = screen_times_ms[curScreen];
    if(current_screen_time_ms == 0) return;  // user disabled this screen, don't care what it thinks about the display time

    int specificTime = ((Screen*)slideShow->get_view(curScreen))->desired_display_time();
    if(specificTime > current_screen_time_ms || specificTime == DISP_TIME_DONT_SHOW) {
        current_screen_time_ms = specificTime;
    }
}

void go_to_next_screen(transition_type_t transition) {
    current_screen_time_ms = 0;
    while(current_screen_time_ms == 0) {
        curScreen = (MainViewId_t) (((uint16_t) curScreen) + 1);
        if(curScreen == VIEW_MAX) curScreen = VIEW_CLOCK;
        update_screen_specific_time();
    }
    slideShow->switch_to(curScreen, transition);
    lastScreenSwitch = xTaskGetTickCount();
}

void go_to_prev_screen(transition_type_t transition) {
    current_screen_time_ms = 0;
    while(current_screen_time_ms == 0) {
        if(curScreen == VIEW_CLOCK) curScreen = (MainViewId_t) (VIEW_MAX - 1);
        else curScreen = (MainViewId_t) (((uint16_t) curScreen) - 1);
        update_screen_specific_time();
    }
    slideShow->switch_to(curScreen, transition);
    lastScreenSwitch = xTaskGetTickCount();
}

void change_screen_if_needed() {
    TickType_t now = xTaskGetTickCount();
    update_screen_specific_time();
    if(now - lastScreenSwitch >= pdMS_TO_TICKS(current_screen_time_ms)) {
        go_to_next_screen((transition_type_t) prefs_get_int(PREFS_KEY_TRANSITION_TYPE));
    }
}

void app_idle_draw(FantaManipulator* graph) {
    graph->clear();
    mainView->render(graph);
}

bool ignoring_keys = false;

void scroll_down() {
    go_to_next_screen(TRANSITION_SLIDE_VERTICAL_UP);
    beepola->beep_blocking(CHANNEL_NOTICE, 1000, 10);
}

void scroll_up() {
    go_to_prev_screen(TRANSITION_SLIDE_VERTICAL_DOWN);
    beepola->beep_blocking(CHANNEL_NOTICE, 1000, 10);
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

    if(!ignoring_keys) {
        if(hid_test_key_state(KEY_DOWN)) {
            scroll_down();
            ignoring_keys = true;
        }
        else if(hid_test_key_state(KEY_UP)) {
            scroll_up();
            ignoring_keys = true;
        }
        else if(hid_test_key_state(KEY_LEFT)) {
            //change_state(STATE_MENU);
            beepola->beep_blocking(CHANNEL_NOTICE, 1000, 10);
            ignoring_keys = true;
        }
        else {
            change_screen_if_needed();
            touchArrows->active = hid_test_key_any(KEYMASK_ALL) != KEYSTATE_RELEASED;
        }
    } else {
        ignoring_keys = (hid_test_key_any(KEY_UP | KEY_DOWN | KEY_LEFT) != KEYSTATE_RELEASED);
        if(hid_test_key_state_repetition(KEY_DOWN)) scroll_down();
        else if(hid_test_key_state_repetition(KEY_UP)) scroll_up();
    }

    hourly_chime();

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);
    hourly_chime_on = prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON);
}