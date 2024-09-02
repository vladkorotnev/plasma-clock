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
#include <views/weather/current_weather.h>
#include <views/weather/daily_forecast.h>
#include <views/weather/chart_precipitation.h>
#include <views/weather/chart_pressure.h>
#include <views/idle_screens/word_of_the_day.h>
#include <views/idle_screens/fb2k.h>
#include <views/idle_screens/next_alarm.h>
#include <views/overlays/signal_icon.h>
#include <views/overlays/touch_arrows_ovl.h>
#include <input/keys.h>
#include <state.h>

static char LOG_TAG[] = "APL_IDLE";

typedef enum MainViewId: uint16_t {
    VIEW_CLOCK = 0,
    VIEW_NEXT_ALARM,
#if HAS(TEMP_SENSOR)
    VIEW_INDOOR_WEATHER,
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    VIEW_REMOTE_WEATHER,
#endif
    VIEW_OUTDOOR_WEATHER,
    VIEW_WEATHER_FORECAST,
    VIEW_WEATHER_PRECIPITATION_CHART,
    VIEW_WEATHER_PRESSURE_CHART,
#if HAS(WORDNIK_API)
    VIEW_WORD_OF_THE_DAY,
#endif
    VIEW_FB2K,

    VIEW_MAX
} MainViewId_t;

static int screen_times_ms[VIEW_MAX] = {0};

int current_screen_time_ms = 0;

static bool did_prepare = false;

static Beeper * beepola;
static NewSequencer * sequencer;
static Yukkuri * yukkuri = nullptr;
static SensorPool * sensors;

static Renderable * mainView;

static SimpleClock * clockView;
static NextAlarmView * nextAlarmView;

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
static DailyForecastView * forecastView;
static WeatherPrecipitationChart * precipitationView;
static WeatherPressureChart * pressureView;
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

static int last_chimed_hour = 0;

void sound_tick_tock() {
    tk_time_of_day_t now = get_current_time_precise();
    if(now.millisecond >= 250 && !tick_tock) {
        if(!sequencer->is_sequencing()) {
            beepola->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
        }
        tick_tock = true;
    } else if (now.millisecond < 250 && tick_tock) {
        tick_tock = false;
    }
}

void _play_hourly_chime_if_enabled(bool first_chime) {
    if(!prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON)) return;
    int melody_no = first_chime ? prefs_get_int(PREFS_KEY_FIRST_CHIME_MELODY) : prefs_get_int(PREFS_KEY_HOURLY_CHIME_MELODY);
    const melody_sequence_t * melody = melody_from_no(melody_no);
    sequencer->play_sequence(melody, SEQUENCER_NO_REPEAT);
}

void hourly_chime() {
    tk_time_of_day now = get_current_time_coarse();
    int first_hour = prefs_get_int(PREFS_KEY_HOURLY_CHIME_START_HOUR);
    if(now.hour != last_chimed_hour) {
        last_chimed_hour = now.hour;
        bool first_chime = (now.hour == first_hour);
        if(  now.hour >= first_hour
          && now.hour <= prefs_get_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR)
        ) {
#if !HAS(AQUESTALK)
            _play_hourly_chime_if_enabled(first_chime);
#else
            if(!prefs_get_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR)) {
                // Voice chime disabled, just play normal chime
                _play_hourly_chime_if_enabled(first_chime);
            } else {
                // Voice chime enabled, speak hour then play chime
                static char hourBuf[64] = { 0 };
                if(now.minute == 0) {
                    snprintf(hourBuf, sizeof(hourBuf), "<NUMK VAL=%i COUNTER=ji>;de_su,", now.hour);
                } else {
                    snprintf(hourBuf, sizeof(hourBuf), "<NUMK VAL=%i COUNTER=ji>,<NUMK VAL=%i COUNTER=funn>;de_su,", now.hour, now.minute);
                }
                YukkuriUtterance hourUtterance = YukkuriUtterance(hourBuf, [first_chime](bool) {
                    _play_hourly_chime_if_enabled(first_chime);
                });

                if(first_chime && prefs_get_bool(PREFS_KEY_VOICE_ANNOUNCE_DATE)) {
                    // Also need date
                    static char dateBuf[64] = {0};
                    tk_date_t today = get_current_date();
                    snprintf(dateBuf, sizeof(dateBuf), "<NUMK VAL=%i COUNTER=gatsu>;<NUMK VAL=%i COUNTER=nichi>", today.month, today.day);
                    yukkuri->speak(dateBuf);
                }

                yukkuri->speak(hourUtterance);
            }
#endif
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

void app_idle_prepare(SensorPool* s, Beeper* b, NewSequencer* seq, Yukkuri* tts) {
    if(did_prepare) return;

    did_prepare = true;
    beepola = b;
    sequencer = seq;
    sensors = s;
    yukkuri = tts;

    tk_time_of_day now = get_current_time_coarse();
    last_chimed_hour = now.hour;

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);

    screen_times_ms[VIEW_CLOCK] = prefs_get_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS) * 1000;
    screen_times_ms[VIEW_NEXT_ALARM] = prefs_get_int(PREFS_KEY_SCRN_TIME_NEXT_ALARM_SECONDS) * 1000;
#if HAS(TEMP_SENSOR)
    screen_times_ms[VIEW_INDOOR_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS) * 1000;
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    screen_times_ms[VIEW_REMOTE_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS) * 1000;
#endif
    screen_times_ms[VIEW_OUTDOOR_WEATHER] = prefs_get_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS) * 1000;
    screen_times_ms[VIEW_WEATHER_FORECAST] = prefs_get_int(PREFS_KEY_SCRN_TIME_FORECAST_SECONDS) * 1000;
    screen_times_ms[VIEW_WEATHER_PRECIPITATION_CHART] = prefs_get_int(PREFS_KEY_SCRN_TIME_PRECIPITATION_SECONDS) * 1000;
    screen_times_ms[VIEW_WEATHER_PRESSURE_CHART] = prefs_get_int(PREFS_KEY_SCRN_TIME_PRESSURE_SECONDS) * 1000;
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
    rain = new RainOverlay(HWCONF_DISPLAY_WIDTH_PX, HWCONF_DISPLAY_HEIGHT_PX);
    thunder = new ThunderOverlay(HWCONF_DISPLAY_WIDTH_PX, HWCONF_DISPLAY_HEIGHT_PX);
    signalIndicator = new SignalStrengthIcon(sensors);
    weatherView = new CurrentWeatherView();
    forecastView = new DailyForecastView();
    precipitationView = new WeatherPrecipitationChart();
    pressureView = new WeatherPressureChart();
    fb2kView = new Fb2kView();
    nextAlarmView = new NextAlarmView();

    touchArrows = new TouchArrowOverlay();
    touchArrows->bottom = true;
    touchArrows->top = true;
    touchArrows->left = true;

    // thunder hurts readability on other views, so keep it on clock only
    ScreenCompositor * thunderClock = new ScreenCompositor(clockView);
    if(prefs_get_bool(PREFS_KEY_WEATHER_OVERLAY))
        thunderClock->add_layer(thunder);

    slideShow = new ViewMultiplexor();
    slideShow->add_view(thunderClock, VIEW_CLOCK);
    slideShow->add_view(nextAlarmView, VIEW_NEXT_ALARM);
#if HAS(TEMP_SENSOR)
    indoorView = new IndoorView(sensors);
    slideShow->add_view(indoorView, VIEW_INDOOR_WEATHER);
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
    remoteWeatherView = new WoSensorView(sensors);
    slideShow->add_view(remoteWeatherView, VIEW_REMOTE_WEATHER);
#endif
    slideShow->add_view(weatherView, VIEW_OUTDOOR_WEATHER);
    slideShow->add_view(forecastView, VIEW_WEATHER_FORECAST);
    slideShow->add_view(precipitationView, VIEW_WEATHER_PRECIPITATION_CHART);
    slideShow->add_view(pressureView, VIEW_WEATHER_PRESSURE_CHART);
#if HAS(WORDNIK_API)
    wotdView = new WordOfTheDayView();
    slideShow->add_view(wotdView, VIEW_WORD_OF_THE_DAY);
#endif
    slideShow->add_view(fb2kView, VIEW_FB2K);

    lastScreenSwitch = xTaskGetTickCount();

    ViewCompositor * rainyClock = new ViewCompositor();
    rainyClock->add_layer(slideShow);
    rainyClock->add_layer(signalIndicator);
    if(prefs_get_bool(PREFS_KEY_WEATHER_OVERLAY))
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

    if(hid_test_key_state_repetition(KEY_DOWN)) {
        go_to_next_screen(TRANSITION_SLIDE_VERTICAL_UP);
    }
    else if(hid_test_key_state_repetition(KEY_UP)) {
        go_to_prev_screen(TRANSITION_SLIDE_VERTICAL_DOWN);
    }
    else if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        push_state(STATE_MENU, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
    }
    else {
        change_screen_if_needed();
        key_state_t anykey = hid_test_key_any(KEYMASK_ALL);
        touchArrows->active = anykey != KEYSTATE_RELEASED;
        if(anykey != KEYSTATE_RELEASED) {
            sequencer->stop_sequence();
        }
    }

    hourly_chime();

    tick_tock_enable = prefs_get_bool(PREFS_KEY_TICKING_SOUND);

#if HAS(BALANCE_BOARD_INTEGRATION)
    if(sensor_info_t * info = sensors->get_info(SENSOR_ID_BALANCE_BOARD_STARTLED)) {
        if(info->last_result) {
            push_state(STATE_WEIGHING, TRANSITION_SLIDE_HORIZONTAL_LEFT);
        }
    }
#endif
}
