#include "network/admin_panel.h"
#include <service/prefs.h>
#include <service/owm/weather.h>
#include <views/transitions.h>
#include <sound/melodies.h>
#include <GyverPortal.h>
#include <Arduino.h>

static char LOG_TAG[] = "ADMIN";
static TaskHandle_t hTask = NULL;
static GyverPortal ui;
static SensorPool *sensors;
static Beeper *beeper;

extern "C" void AdminTaskFunction( void * pvParameter );

void AdminTaskFunction( void * pvParameter )
{
    while(1) {
        ui.tick();
        vTaskDelay(100);
    }
}

GP_BUTTON reboot_btn("reboot", "Save & Restart PIS-OS");

static void render_bool(const char * title, prefs_key_t key) {
    GP.LABEL(title);
    GP.SWITCH(key, prefs_get_bool(key));
}

static void save_bool(prefs_key_t key) {
    bool temp = false;
    if(ui.clickBool(key, temp)) {
        prefs_set_bool(key, temp);
        beeper->beep_blocking(CHANNEL_NOTICE, 1000, 50);
    }
}

static void render_int(const char * title, prefs_key_t key) {
    GP.LABEL(title);
    GP.NUMBER(key, "", prefs_get_int(key));
}

static void save_int(prefs_key_t key, int min, int max) {
    int temp;
    if(ui.clickInt(key, temp)) {
        ESP_LOGV(LOG_TAG, "Save int %s: min %i, max %i, val %i", key, min, max, temp);
        temp = std::min(temp, max);
        temp = std::max(temp, min);
        prefs_set_int(key, temp);
        beeper->beep_blocking(CHANNEL_NOTICE, 1000, 50);
    }
}

static void render_string(const char * title, prefs_key_t key, bool is_pass) {
    GP.LABEL(title);
    if(is_pass) {
        GP.PASS_EYE(key, title, prefs_get_string(key));
    } else {
        GP.TEXT(key, title, prefs_get_string(key));
    }
}

static void save_string(prefs_key_t key) {
    String temp;
    if(ui.clickString(key, temp)) {
        prefs_set_string(key, temp);
        beeper->beep_blocking(CHANNEL_NOTICE, 1000, 50);
    }
}


void build() {
    GP.BUILD_BEGIN();
    GP.PAGE_TITLE("PIS-OS Admin Panel");
    GP.THEME(GP_DARK);
    GP.JQ_SUPPORT();

    GP.TITLE("PIS-OS Admin Panel");

    GP.SPOILER_BEGIN("WiFi", GP_BLUE);
        render_string("SSID", PREFS_KEY_WIFI_SSID, false);
        render_string("Password", PREFS_KEY_WIFI_PASS, true);
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Clock", GP_BLUE);
        render_bool("Ticking sound:", PREFS_KEY_TICKING_SOUND);
        render_bool("No sound when screen is off:", PREFS_KEY_NO_SOUND_WHEN_OFF);
        GP.HR();
        render_bool("Hourly chime:", PREFS_KEY_HOURLY_CHIME_ON);
        GP.SELECT(PREFS_KEY_HOURLY_CHIME_MELODY, all_chime_names, prefs_get_int(PREFS_KEY_HOURLY_CHIME_MELODY));
        GP.BREAK();
        render_int("From:", PREFS_KEY_HOURLY_CHIME_START_HOUR);
        render_int("To:", PREFS_KEY_HOURLY_CHIME_STOP_HOUR);
        GP.HR();
        GP.LABEL("Screen transition:");
        GP.SELECT(PREFS_KEY_TRANSITION_TYPE, "Off,Wipe,Horizontal Slide,Vertical Slide,Random", prefs_get_int(PREFS_KEY_TRANSITION_TYPE));
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Display", GP_BLUE);
        render_int("Show clock for [s]:", PREFS_KEY_SCRN_TIME_CLOCK_SECONDS);
        GP.BREAK();
        render_int("Show temperature for [s]:", PREFS_KEY_SCRN_TIME_INDOOR_SECONDS);
        GP.BREAK();
        render_int("Show current weather for [s]:", PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS);
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Sensors", GP_BLUE);
    GP.JQ_UPDATE_BEGIN(1000);
        sensor_info_t* sens;

        GP.LABEL("Light sensor: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_LIGHT);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result), "light_val");
        } else {
            GP.LABEL("----", "light_val");
        }

        GP.LABEL("Motion sensor: ");
        sens = sensors->get_info(SENSOR_ID_MOTION);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result), "mot_val");
        } else {
            GP.LABEL("?", "mot_val");
        }

        GP.LABEL("Temperature: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_TEMPERATURE);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "temp_val");
        } else {
            GP.LABEL("----", "temp_val");
        }

        GP.LABEL("Humidity: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_HUMIDITY);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "hum_val");
        } else {
            GP.LABEL("----", "hum_val");
        }
    GP.JQ_UPDATE_END();
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Power Management", GP_BLUE);
        render_int("Bright screen when over:", PREFS_KEY_LIGHTNESS_THRESH_UP);
        render_int("Dark screen when under:", PREFS_KEY_LIGHTNESS_THRESH_DOWN);
        GP.HR();
        render_int("Turn off display after seconds:", PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS);
        render_int("Shut down high voltage after more seconds:", PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS);
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("OpenWeatherMap", GP_BLUE);
        render_string("Latitude", PREFS_KEY_WEATHER_LAT, false);
        render_string("Longitude", PREFS_KEY_WEATHER_LON, false);
        render_string("API Key", PREFS_KEY_WEATHER_APIKEY, true);
        render_int("Update interval, minutes:", PREFS_KEY_WEATHER_INTERVAL_MINUTES);

        current_weather_t weather;
        if(weather_get_current(&weather)) {
            GP.HR();
            GP.LABEL("Current weather:");
            GP.LABEL(String(kelvin_to(weather.temperature_kelvin, CELSIUS)) + " C");
            GP.LABEL(String(weather.humidity_percent) + " %");
            GP.LABEL(String(weather.pressure_hpa) + "hPa");
        }
#ifdef DEMO_WEATHER_WEBADMIN
    GP.LABEL("Change weather:");
    GP.NUMBER("demo_weather", "Demo weather code", 200);
#endif
    GP.SPOILER_END();

    GP.HR();
#if defined(PDFB_PERF_LOGS)
    render_bool("FPS counter", PREFS_KEY_FPS_COUNTER);
#endif
    GP.BUTTON(reboot_btn);
    GP.BUILD_END();
}

void action() {
    if(ui.click()) {
        save_string(PREFS_KEY_WIFI_SSID);
        save_string(PREFS_KEY_WIFI_PASS);
        save_bool(PREFS_KEY_TICKING_SOUND);
        save_bool(PREFS_KEY_HOURLY_CHIME_ON);
        save_int(PREFS_KEY_HOURLY_CHIME_START_HOUR, 0, 23);
        save_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 0, 23);
        save_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS, 1, 3600);
        save_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS, 1, 3600);
        save_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS, 1, 3600);
        save_bool(PREFS_KEY_NO_SOUND_WHEN_OFF);
        save_int(PREFS_KEY_TRANSITION_TYPE, TRANSITION_NONE, TRANSITION_RANDOM);
        save_int(PREFS_KEY_LIGHTNESS_THRESH_UP, 0, 4096);
        save_int(PREFS_KEY_LIGHTNESS_THRESH_DOWN, 0, 4096);
        save_int(PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS, 60, 21600);
        save_int(PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS, 60, 21600);
        save_string(PREFS_KEY_WEATHER_APIKEY);
        save_string(PREFS_KEY_WEATHER_LAT);
        save_string(PREFS_KEY_WEATHER_LON);
        save_int(PREFS_KEY_WEATHER_INTERVAL_MINUTES, 30, 24 * 60);
        save_bool(PREFS_KEY_FPS_COUNTER);

#ifdef DEMO_WEATHER_WEBADMIN
        int temp_wc;
        if(ui.clickInt("demo_weather", temp_wc)) {
            current_weather_t w;
            weather_get_current(&w);
            w.conditions = (weather_condition_t) temp_wc;
            w.temperature_kelvin = 294.15;
            w.last_updated = xTaskGetTickCount();
            strncpy(w.description, "Very nice day today. Feels like having a good time.", 64);
            weather_set_demo(&w);
        }
#endif

        int temp_chime;
        if(ui.clickInt(PREFS_KEY_HOURLY_CHIME_MELODY, temp_chime)) {
            temp_chime = std::min(temp_chime, all_chime_count);
            temp_chime = std::max(temp_chime, 0);

            ESP_LOGV(LOG_TAG, "Preview chime #%i", temp_chime);
            
            prefs_set_int(PREFS_KEY_HOURLY_CHIME_MELODY, temp_chime);

            if(temp_chime == all_chime_count) {
                temp_chime = esp_random() % all_chime_count;
            }
            
            melody_sequence_t melody = all_chime_list[temp_chime];
            BeepSequencer * s = new BeepSequencer(beeper);
            s->play_sequence(melody, CHANNEL_NOTICE, 0);
            s->wait_end_play();
            delete s;
        }

        if(ui.click(reboot_btn)) {
            prefs_force_save();
            BeepSequencer * s = new BeepSequencer(beeper);
            s->play_sequence(tulula_fvu, CHANNEL_NOTICE, 0);
            s->wait_end_play();
            ESP.restart();
        }
    }
}

void admin_panel_prepare(SensorPool* s, Beeper* b) {
    sensors = s;
    beeper = b;
    ui.attachBuild(build);
    ui.attach(action);
#if defined(ADMIN_LOGIN) && defined(ADMIN_PASS)
    ui.enableAuth(ADMIN_LOGIN, ADMIN_PASS);
#endif
    ui.start();

    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        AdminTaskFunction,
        "ADM",
        4096,
        nullptr,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}