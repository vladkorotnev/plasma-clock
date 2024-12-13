#include "network/admin_panel.h"
#include <LittleFS.h>
#include <device_config.h>
#include <network/netmgr.h>
#include <backup.h>
#include <service/prefs.h>
#include <service/owm/weather.h>
#include <service/wordnik.h>
#include <service/alarm.h>
#include <views/transitions/transitions.h>
#include <input/keys.h>
#include <sound/melodies.h>
#include <GyverPortal.h>
#include <Arduino.h>
#include <os_config.h>
#include <state.h>

static char LOG_TAG[] = "ADMIN";
static TaskHandle_t hTask = NULL;
static GyverPortal ui(&LittleFS);
static SensorPool *sensors;
static Beeper *beeper;
static std::string all_chime_names_csv = "";

extern "C" void AdminTaskFunction( void * pvParameter );

void AdminTaskFunction( void * pvParameter )
{
    while(1) {
        ui.tick();
        vTaskDelay(100);
    }
}

GP_BUTTON reboot_btn("reboot", "Save & Restart " PRODUCT_NAME);

static void render_bool(const char * title, prefs_key_t key) {
    GP.LABEL(title);
    GP.SWITCH(key, prefs_get_bool(key));
}

static void save_bool(prefs_key_t key) {
    bool temp = false;
    if(ui.clickBool(key, temp)) {
        prefs_set_bool(key, temp);
        beeper->beep(CHANNEL_NOTICE, 1000, 50);
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
        beeper->beep(CHANNEL_NOTICE, 1000, 50);
    }
}

static void render_string(const char * title, prefs_key_t key, bool is_pass = false) {
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
        beeper->beep(CHANNEL_NOTICE, 1000, 50);
    }
}

static void render_melody(const char * title, prefs_key_t key) {
    GP.LABEL(title);
    GP.SELECT(key, all_chime_names_csv.c_str(), prefs_get_int(key));
}

static void save_melody(prefs_key_t key) {
    int temp_chime;
    if(ui.clickInt(key, temp_chime)) {
        temp_chime = std::min(temp_chime, all_chime_count);
        temp_chime = std::max(temp_chime, 0);

        ESP_LOGV(LOG_TAG, "Preview chime #%i for %s", temp_chime, key);
        
        prefs_set_int(key, temp_chime);
    }
}

const char * days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

static void render_alarms() {
    GP.FORM_BEGIN("/alarms");
    GP.SPAN("Don't forget to save after editing! If no day is selected and alarm is enabled, it will only trigger once.");
    GP.BREAK();
    const alarm_setting_t * alarms = get_alarm_list();
    for(int i = 0; i < ALARM_LIST_SIZE; i++) {
        if(i > 0) GP.HR();

        const alarm_setting_t a = alarms[i];
        String tmp = String("Alarm ");
        tmp += (i + 1);
        GP.LABEL(tmp);
        GP.BREAK();
        
        tmp = String("alm_") + i + "_";
        
        GP.LABEL("Enabled: ");
        GP.CHECK(tmp + "enable", a.enabled);
        GP.BREAK();

        GP.TABLE_BEGIN();
        GP.TR();
        for(int d = 0; d < 7; d++) {
                GP.TD();
                GP.LABEL(days[d]);
        }
        GP.TR();
         for(int d = 0; d < 7; d++) {
                GP.TD();
                GP.CHECK(tmp + days[d], ALARM_ON_DAY(a, d));
        }
        GP.TABLE_END();
        GP.BREAK();

        GP.TABLE_BEGIN();
        GP.TR();
        GP.TD();
        GP.NUMBER(tmp + "h", "8", a.hour);
        GP.TD();
        GP.LABEL(":");
        GP.TD();
        GP.NUMBER(tmp + "m", "30", a.minute);
        GP.TABLE_END();
        GP.BREAK();

        if (sensors->exists(SENSOR_ID_MOTION)) {
            GP.LABEL("Smart: ");
            GP.CHECK(tmp + "smart", a.smart);
            GP.NUMBER(tmp + "margin", "10", a.smart_margin_minutes);
            GP.BREAK();
        }

        GP.SELECT(tmp + "melo", all_chime_names_csv.c_str(), a.melody_no);
    }

    GP.FORM_SEND("Save Alarms");
    GP.FORM_END();
}

static bool save_alarms() {
    if(!ui.form("/alarms")) return false;
    ESP_LOGI(LOG_TAG, "Save alarms");
    for(int i = 0; i < ALARM_LIST_SIZE; i++) {
        alarm_setting_t a = {0};
        String tmp = String("alm_") + i + "_";

        a.enabled = ui.getBool(tmp + "enable");
        a.smart = ui.getBool(tmp + "smart");

        for(int d = 0; d < 7; d++) {
            if(ui.getBool(tmp + days[d])) {
                a.days |= ALARM_DAY_OF_WEEK(d);
            } else {
                a.days &= ~ALARM_DAY_OF_WEEK(d);
            }
        }
        a.hour = ui.getInt(tmp + "h");
        a.minute = ui.getInt(tmp + "m");
        a.melody_no = ui.getInt(tmp + "melo");
        a.smart_margin_minutes = ui.getInt(tmp + "margin");

        set_alarm(i, a);
    }
    beeper->beep(CHANNEL_NOTICE, 1000, 50);
    return true;
}

static inline bool _rmc_key(const char * key, key_id_t id) {
    if(ui.clickDown(key)) {
        hid_set_key_state(id, true);
        return true;
    }
    else if(ui.clickUp(key)) {
        hid_set_key_state(id, false);
        return true;
    }
    return false;
}

static bool process_remote() {
    if(ui.hold()) {
        bool rslt = false;
        rslt |= _rmc_key("rmc_up", KEY_UP);
        rslt |= _rmc_key("rmc_down", KEY_DOWN);
        rslt |= _rmc_key("rmc_left", KEY_LEFT);
        rslt |= _rmc_key("rmc_right", KEY_RIGHT);
        rslt |= _rmc_key("rmc_headpat", KEY_HEADPAT);
        return rslt;
    }
    return false;
}

static void build() {
    GP.BUILD_BEGIN_FILE();
    GP.PAGE_TITLE(PRODUCT_NAME " Admin Panel " PRODUCT_VERSION);
    GP.THEME_FILE("GP_DARK");
    GP.JQ_SUPPORT_FILE();

    GP.TITLE(PRODUCT_NAME " Admin Panel " PRODUCT_VERSION);

    GP.SPOILER_BEGIN("Remote", GP_BLUE);
        GP.BUTTON("rmc_headpat", "Headpat");
        GP.HR();
        GP.TABLE_BEGIN();
            GP.TR();
                GP.TD();
                GP.TD();
                GP.BUTTON("rmc_up", "↑");
                GP.TD();
            GP.TR();
                GP.TD();
                GP.BUTTON("rmc_left", "←");
                GP.TD();
                GP.TD();
                GP.BUTTON("rmc_right", "→");
            GP.TR();
                GP.TD();
                GP.TD();
                GP.BUTTON("rmc_down", "↓");
                GP.TD();
        GP.TABLE_END();
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("WiFi", GP_BLUE);
        render_string("SSID", PREFS_KEY_WIFI_SSID);
        render_string("Password", PREFS_KEY_WIFI_PASS, true);
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Clock", GP_BLUE);
        render_bool("24-hour display:", PREFS_KEY_DISP_24_HRS);
        render_bool("Show seconds:", PREFS_KEY_SHOW_SECONDS);
        render_bool("Blink separators:", PREFS_KEY_BLINK_SEPARATORS);
        render_bool("Ticking sound:", PREFS_KEY_TICKING_SOUND);
        render_bool("Only when screen is on:", PREFS_KEY_NO_SOUND_WHEN_OFF);
        GP.HR();
        render_bool("Hourly chime:", PREFS_KEY_HOURLY_CHIME_ON);
        render_bool("Fake Soviet radio time signals:", PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL);
        GP.BREAK();
        render_melody("First of the day:", PREFS_KEY_FIRST_CHIME_MELODY);
        render_melody("All others:", PREFS_KEY_HOURLY_CHIME_MELODY);
        GP.BREAK();
        render_int("From:", PREFS_KEY_HOURLY_CHIME_START_HOUR);
        render_int("To:", PREFS_KEY_HOURLY_CHIME_STOP_HOUR);
        GP.HR();
        render_bool("Use NTP:", PREFS_KEY_TIMESERVER_ENABLE);
        GP.BREAK();
        render_string("NTP server:", PREFS_KEY_TIMESERVER);
        GP.BREAK();
        render_int("Sync interval [s]:", PREFS_KEY_TIME_SYNC_INTERVAL_SEC);
        GP.BREAK();
        render_string("Timezone descriptor:", PREFS_KEY_TIMEZONE);
        GP.LABEL("E.g. JST-9 or AST4ADT,M3.2.0,M11.1.0. See <a href=\"https://www.iana.org/time-zones\">IANA TZ DB</a> for reference.");
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Alarms", GP_BLUE);
        render_int("Snooze minutes:", PREFS_KEY_ALARM_SNOOZE_MINUTES);
        render_int("Max duration [m]:", PREFS_KEY_ALARM_MAX_DURATION_MINUTES);
        render_alarms();
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Display", GP_BLUE);
        GP.LABEL("Display language (not in WebUI):");
        GP.SELECT(PREFS_KEY_DISP_LANGUAGE, "English,Русский,日本語", prefs_get_int(PREFS_KEY_DISP_LANGUAGE));
        GP.HR();
#if HAS(VARYING_BRIGHTNESS)
        GP.LABEL("Display brightness:");
        GP.SELECT(PREFS_KEY_BRIGHTNESS_MODE,
        "Dim,Bright"
#if HAS(LIGHT_SENSOR)
        ",Automatic"
#endif
        , prefs_get_int(PREFS_KEY_BRIGHTNESS_MODE));
        GP.HR();
#endif
        render_int("Show clock for [s]:", PREFS_KEY_SCRN_TIME_CLOCK_SECONDS);
        GP.BREAK();
        render_int("Show next alarm countdown for [s]:", PREFS_KEY_SCRN_TIME_NEXT_ALARM_SECONDS);
        GP.BREAK();
        #if HAS(TEMP_SENSOR)
        render_int("Show temperature for [s]:", PREFS_KEY_SCRN_TIME_INDOOR_SECONDS);
        GP.BREAK();
        #endif
        #if HAS(SWITCHBOT_METER_INTEGRATION)
        render_int("Show Switchbot temperature for [s]:", PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS);
        GP.BREAK();
        #endif
        render_int("Show current weather for [s]:", PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS);
        GP.BREAK();
        render_int("Show 2-day forecast for [s]:", PREFS_KEY_SCRN_TIME_FORECAST_SECONDS);
        GP.BREAK();
        render_int("Show precipitation % graph for [s]:", PREFS_KEY_SCRN_TIME_PRECIPITATION_SECONDS);
        GP.BREAK();
        render_int("Show pressure graph for [s]:", PREFS_KEY_SCRN_TIME_PRESSURE_SECONDS);
        GP.BREAK();
        #if HAS(WORDNIK_API)
        render_int("Show word of the day for [s]:", PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS);
        GP.BREAK();
        #endif
        render_int("Show Fb2k for [s]:", PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS);
        GP.HR();
        GP.LABEL("Screen transition:");
        GP.SELECT(PREFS_KEY_TRANSITION_TYPE, "Off,Wipe,Horizontal Slide To Left,Horizontal Slide To Right,Vertical Slide Up,Vertical Slide Down,Random", prefs_get_int(PREFS_KEY_TRANSITION_TYPE));
        GP.HR();
        GP.LABEL("Scroll speed:");
        GP.SELECT(PREFS_KEY_DISP_SCROLL_SPEED, "Slow,Medium,Fast,Sonic", prefs_get_int(PREFS_KEY_DISP_SCROLL_SPEED));
        GP.HR();
        render_bool("Use Fahrenheit:", PREFS_KEY_WEATHER_USE_FAHRENHEIT);
        GP.HR();
        render_bool("Keypress beep:", PREFS_KEY_BUTTON_BEEP);
    GP.SPOILER_END();
    GP.BREAK();

#if HAS(AQUESTALK)
    GP.SPOILER_BEGIN("Voice", GP_BLUE);
        render_string("License key:", PREFS_KEY_VOICE_LICENSE, true);
        GP.HR();
        GP.LABEL("Talking clock language:");
        GP.SELECT(PREFS_KEY_TTS_LANGUAGE, "English,Русский,日本語", prefs_get_int(PREFS_KEY_TTS_LANGUAGE));
        GP.HR();
        GP.LABEL("Voice mode:");
        GP.SELECT(PREFS_KEY_VOICE_MODE_RESAMPLING, "Clear,Loud", prefs_get_int(PREFS_KEY_VOICE_MODE_RESAMPLING));
        GP.HR();
        render_int("Speed [1~200]%:", PREFS_KEY_VOICE_SPEED);
        render_bool("Speak hour on chime", PREFS_KEY_VOICE_ANNOUNCE_HOUR);
        render_bool("Speak time on headpat", PREFS_KEY_VOICE_SPEAK_ON_HEADPAT);
        render_bool("24-hour announcements:", PREFS_KEY_VOICE_24_HRS);
        render_bool("Speak date on first chime", PREFS_KEY_VOICE_ANNOUNCE_DATE);
    GP.SPOILER_END();
    GP.BREAK();
#endif

#if HAS(TEMP_SENSOR)
    GP.SPOILER_BEGIN("Calibration", GP_BLUE);
        render_int("Temperature offset, Celsius:", PREFS_KEY_TEMP_SENSOR_TEMP_OFFSET);
        GP.BREAK();
        render_int("Humidity offset:", PREFS_KEY_TEMP_SENSOR_HUM_OFFSET);
    GP.SPOILER_END();
    GP.BREAK();
#endif

    GP.SPOILER_BEGIN("Overlays", GP_BLUE);
        render_bool("FPS counter", PREFS_KEY_FPS_COUNTER);
        GP.LABEL("WiFi status:");
        GP.SELECT(PREFS_KEY_WIFI_ICON, "None,Disconnected only,On display power on,Always", prefs_get_int(PREFS_KEY_WIFI_ICON));
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Sensors", GP_BLUE);
    GP.JQ_UPDATE_BEGIN(10000);
        sensor_info_t* sens;

        GP.LABEL("WiFi: ");
        GP.LABEL(NetworkManager::network_name(), "net_name");
        char buf[16];
        snprintf(buf, 15, "(%i dBm)", NetworkManager::rssi());
        GP.LABEL(buf, "rssi_val");
        GP.BREAK();

        GP.LABEL("Uptime: ");
        tk_time_of_day_t uptime = get_uptime();
        snprintf(buf, 16, "%02d:%02d:%02d", uptime.hour, uptime.minute, uptime.second, uptime.millisecond);
        GP.LABEL(buf, "uptime_val");
        GP.BREAK();

        #if HAS(LIGHT_SENSOR)
        GP.LABEL("Light sensor: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_LIGHT);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result), "light_val");
        } else {
            GP.LABEL("----", "light_val");
        }
        GP.BREAK();
        #endif

        #if HAS(MOTION_SENSOR)
        GP.LABEL("Motion sensor: ");
        sens = sensors->get_info(SENSOR_ID_MOTION);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result), "mot_val");
        } else {
            GP.LABEL("?", "mot_val");
        }
        GP.BREAK();
        #endif

        #if HAS(TEMP_SENSOR)
        GP.LABEL("Temperature: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_TEMPERATURE);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "temp_val");
        } else {
            GP.LABEL("----", "temp_val");
        }
        GP.BREAK();
        

        GP.LABEL("Humidity: ");
        sens = sensors->get_info(SENSOR_ID_AMBIENT_HUMIDITY);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "hum_val");
        } else {
            GP.LABEL("----", "hum_val");
        }
        GP.BREAK();
        #endif

        #if HAS(SWITCHBOT_METER_INTEGRATION)
        GP.LABEL("Remote temperature: ");
        sens = sensors->get_info(SENSOR_ID_SWITCHBOT_INDOOR_TEMPERATURE);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "r_temp_val");
        } else {
            GP.LABEL("----", "r_temp_val");
        }
        GP.BREAK();

        GP.LABEL("Remote humidity: ");
        sens = sensors->get_info(SENSOR_ID_SWITCHBOT_INDOOR_HUMIDITY);
        if(sens != nullptr) {
            GP.LABEL(String(sens->last_result / 100.0), "r_hum_val");
        } else {
            GP.LABEL("----", "r_hum_val");
        }
        GP.BREAK();
        #endif
    GP.JQ_UPDATE_END();
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("Power Management", GP_BLUE);
        #if HAS(VARYING_BRIGHTNESS) && HAS(LIGHT_SENSOR)
        render_int("Bright screen when over:", PREFS_KEY_LIGHTNESS_THRESH_UP);
        render_int("Dark screen when under:", PREFS_KEY_LIGHTNESS_THRESH_DOWN);
        GP.HR();
        #endif
#if HAS(DISPLAY_BLANKING)
        render_int("Blank display after seconds:", PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS);
#endif
        render_int("Turn off display after seconds:", PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS);
    GP.SPOILER_END();
    GP.BREAK();

    GP.SPOILER_BEGIN("OpenWeatherMap", GP_BLUE);
        render_string("Latitude", PREFS_KEY_WEATHER_LAT);
        render_string("Longitude", PREFS_KEY_WEATHER_LON);
        render_string("API Key", PREFS_KEY_WEATHER_APIKEY, true);
        render_int("Update interval [m]:", PREFS_KEY_WEATHER_INTERVAL_MINUTES);
        render_bool("Weather effects on display:", PREFS_KEY_WEATHER_OVERLAY);

        current_weather_t weather;
        if(weather_get_current(&weather)) {
            GP.HR();
            GP.LABEL("Current weather:");
            GP.LABEL(String(convert_temperature(KELVIN, weather.temperature_kelvin)) + " " + (char) preferred_temperature_unit());
            GP.LABEL(String(weather.humidity_percent) + " %");
            GP.LABEL(String(weather.pressure_hpa) + "hPa");
        }
#ifdef DEMO_WEATHER_WEBADMIN
    GP.LABEL("Change weather:");
    GP.NUMBER("demo_weather", "Demo weather code", 200);
#endif
    GP.SPOILER_END();
    GP.BREAK();

#if HAS(WORDNIK_API)
    GP.SPOILER_BEGIN("Wordnik", GP_BLUE);
        render_string("API Key", PREFS_KEY_WORDNIK_APIKEY, true);
        render_int("Update interval [m]:", PREFS_KEY_WORDNIK_INTERVAL_MINUTES);

        char wotd[32];
        char definition[256];
        if(wotd_get_current(wotd, 32, definition, 256)) {
            GP.HR();
            GP.LABEL("Today's word:");
            GP.LABEL(wotd);
            GP.BREAK();
            GP.SPAN(definition);
        }
    GP.SPOILER_END();
    GP.BREAK();
#endif

#if HAS(SWITCHBOT_METER_INTEGRATION)
    GP.SPOILER_BEGIN("Switchbot Meter", GP_BLUE);
        render_bool("Connect to Meter", PREFS_KEY_SWITCHBOT_METER_ENABLE);
        render_bool("Emulate local", PREFS_KEY_SWITCHBOT_EMULATES_LOCAL);
        GP.SPAN("When local sensor is not available, forwards the Meter data in place of that.");
        render_string("Meter MAC address:", PREFS_KEY_SWITCHBOT_METER_MAC);
        GP.SPAN("Use the Device Info tab in the Switchbot app to find it");
    GP.SPOILER_END();
    GP.BREAK();
#endif

    GP.SPOILER_BEGIN("Foobar2000", GP_BLUE);
        render_string("Control Server IP", PREFS_KEY_FOOBAR_SERVER);
        render_int("Control Server Port:", PREFS_KEY_FOOBAR_PORT);
        GP.SPAN("Please set the format in foo_controlserver to: %artist%|%title%, and main delimiter to: |");
    GP.SPOILER_END();
    GP.BREAK();

    // GP.SPOILER_BEGIN("Music Files", GP_BLUE);
    //     GP.FILE_UPLOAD("music_upload", "Upload Music", ".pomf", GP_BLUE);
    //     GP.FILE_MANAGER(&LittleFS, "/music");
    // GP.SPOILER_END();
    // GP.BREAK();

    GP.SPOILER_BEGIN("Administration", GP_BLUE);
        render_bool("Remote control server", PREFS_KEY_REMOTE_SERVER);
        render_bool("Serial MIDI input", PREFS_KEY_SERIAL_MIDI);
#if HAS(HTTPFVU)
        render_bool("Check for updates automatically", PREFS_KEY_FVU_AUTO_CHECK);
        render_bool("Install updates automatically", PREFS_KEY_FVU_AUTO_INSTALL);
        render_int("Update check interval, minutes", PREFS_KEY_FVU_AUTO_CHECK_INTERVAL_MINUTES);
        render_string("Firmware update server", PREFS_KEY_FVU_SERVER);
#endif
        render_string("Webadmin login", PREFS_KEY_ADMIN_ID);
        render_string("Webadmin password", PREFS_KEY_ADMIN_PASS);
        GP.BUTTON_DOWNLOAD("prefs_backup.bin", "Settings backup", GP_BLUE);
        GP.BUTTON_DOWNLOAD("crashdump.elf", "Last crash dump", GP_BLUE);
        GP.FILE_UPLOAD_RAW("prefs_restore", "Settings restore", GP_BLUE, "", "", "/prefs_restore");
    GP.SPOILER_END();

    GP.HR();
    GP.BUTTON(reboot_btn);
    GP.BUILD_END();
}

static char binary_mime[] = "application/octet-stream";
static char png_mime[] = "image/png";
void downloadPartition(const void * partition, size_t size) {
    ui.server.send_P(200, binary_mime, (const char*) partition, size);
} 
void action() {
    if(process_remote()) return;
    if(save_alarms()) return;
    if(ui.click()) {
        save_int(PREFS_KEY_ALARM_SNOOZE_MINUTES, 0, 30);
        save_int(PREFS_KEY_ALARM_MAX_DURATION_MINUTES, 0, 120);
        save_string(PREFS_KEY_WIFI_SSID);
        save_string(PREFS_KEY_WIFI_PASS);
        save_bool(PREFS_KEY_DISP_24_HRS);
        save_bool(PREFS_KEY_VOICE_24_HRS);
        save_bool(PREFS_KEY_BLINK_SEPARATORS);
        save_bool(PREFS_KEY_SHOW_SECONDS);
        save_bool(PREFS_KEY_TICKING_SOUND);
        save_bool(PREFS_KEY_HOURLY_CHIME_ON);
        save_int(PREFS_KEY_HOURLY_CHIME_START_HOUR, 0, 23);
        save_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 0, 23);
        save_bool(PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL);
        save_melody(PREFS_KEY_FIRST_CHIME_MELODY);
        save_melody(PREFS_KEY_HOURLY_CHIME_MELODY);
        save_bool(PREFS_KEY_TIMESERVER_ENABLE);
        save_string(PREFS_KEY_TIMESERVER);
        save_string(PREFS_KEY_TIMEZONE);
        save_int(PREFS_KEY_TIME_SYNC_INTERVAL_SEC, 600, 21600);
        save_int(PREFS_KEY_SCRN_TIME_NEXT_ALARM_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_CLOCK_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_INDOOR_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_FORECAST_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_PRECIPITATION_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_PRESSURE_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS, 0, 3600);
        save_int(PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS, 0, 3600);
        save_bool(PREFS_KEY_NO_SOUND_WHEN_OFF);
        save_int(PREFS_KEY_TRANSITION_TYPE, TRANSITION_NONE, TRANSITION_RANDOM);
        save_int(PREFS_KEY_DISP_SCROLL_SPEED, 0, 4);
        save_int(PREFS_KEY_BRIGHTNESS_MODE, 0, BRIGHTNESS_MAX_INVALID - 1);
        save_bool(PREFS_KEY_WEATHER_USE_FAHRENHEIT);
        save_bool(PREFS_KEY_BUTTON_BEEP);
        save_int(PREFS_KEY_TEMP_SENSOR_TEMP_OFFSET, -50, 50);
        save_int(PREFS_KEY_TEMP_SENSOR_HUM_OFFSET, -50, 50);
        save_int(PREFS_KEY_LIGHTNESS_THRESH_UP, 0, 4096);
        save_int(PREFS_KEY_LIGHTNESS_THRESH_DOWN, 0, 4096);
        save_int(PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS, 0, 36000);
        save_int(PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS, 0, 72000);
        save_string(PREFS_KEY_WEATHER_APIKEY);
        save_string(PREFS_KEY_WEATHER_LAT);
        save_string(PREFS_KEY_WEATHER_LON);
        save_bool(PREFS_KEY_WEATHER_OVERLAY);
        save_int(PREFS_KEY_WEATHER_INTERVAL_MINUTES, 30, 24 * 60);
        save_string(PREFS_KEY_WORDNIK_APIKEY);
        save_int(PREFS_KEY_WORDNIK_INTERVAL_MINUTES, 60, 3600);
        save_string(PREFS_KEY_FOOBAR_SERVER);
        save_int(PREFS_KEY_FOOBAR_PORT, 1000, 9999);
        save_bool(PREFS_KEY_FPS_COUNTER);
        save_int(PREFS_KEY_WIFI_ICON, 0, 3);
        save_bool(PREFS_KEY_SWITCHBOT_METER_ENABLE);
        save_string(PREFS_KEY_SWITCHBOT_METER_MAC);
        save_bool(PREFS_KEY_SWITCHBOT_EMULATES_LOCAL);
        save_bool(PREFS_KEY_REMOTE_SERVER);
        save_bool(PREFS_KEY_SERIAL_MIDI);
        save_string(PREFS_KEY_VOICE_LICENSE);
        save_int(PREFS_KEY_VOICE_SPEED, 1, 200);
        save_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR);
        save_bool(PREFS_KEY_VOICE_ANNOUNCE_DATE);
        save_bool(PREFS_KEY_VOICE_SPEAK_ON_HEADPAT);
        save_int(PREFS_KEY_DISP_LANGUAGE, 0, 1);
        save_int(PREFS_KEY_TTS_LANGUAGE, 0, 2);
        save_int(PREFS_KEY_VOICE_MODE_RESAMPLING, 0, 1);
        save_bool(PREFS_KEY_FVU_AUTO_CHECK);
        save_bool(PREFS_KEY_FVU_AUTO_INSTALL);
        save_int(PREFS_KEY_FVU_AUTO_CHECK_INTERVAL_MINUTES, 1, 60 * 24);
        save_string(PREFS_KEY_FVU_SERVER);
        save_string(PREFS_KEY_ADMIN_ID);
        save_string(PREFS_KEY_ADMIN_PASS);

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

        if(ui.click(reboot_btn)) {
            prefs_force_save();
            change_state(STATE_RESTART, TRANSITION_NONE);
        }
        return;
    }

    if(ui.download()) {
        const void * ptrPart;
        partition_handle_t hPart;
        size_t szPart;

        if(ui.uri().endsWith("prefs_backup.bin")) {
            if(mount_settings(&ptrPart, &hPart, &szPart)) {
                downloadPartition(ptrPart, szPart);
                unmount_partition(hPart);
            }
        }
        else if(ui.uri().endsWith("crashdump.elf")) {
            // MEMO: read with
            // python %IDF_PATH%\components\espcoredump\espcoredump.py info_corefile --core R:\crashdump.elf .pio\build\bigclock-nodemcu-32s\firmware.elf
            if(mount_crash(&ptrPart, &hPart, &szPart)) {
                downloadPartition(ptrPart, szPart);
                unmount_partition(hPart);
            }
        }
        else if(LittleFS.exists(ui.uri())) ui.sendFile(LittleFS.open(ui.uri(), "r"));
    }
}

bool prefs_uploading = false;

void admin_panel_prepare(SensorPool* s, Beeper* b) {
    sensors = s;
    beeper = b;
    ui.attachBuild(build);
    ui.attach(action);
    ui.downloadAuto(false);
    ui.uploadAuto(false);

    String login = prefs_get_string(PREFS_KEY_ADMIN_ID);
    String pass = prefs_get_string(PREFS_KEY_ADMIN_PASS);
    if(login.length() > 0 && pass.length() > 0) {
        char * login_raw = (char*) malloc(login.length() + 1);
        char * pass_raw = (char*) malloc(pass.length() + 1);
        strncpy(login_raw, login.c_str(), login.length() + 1);
        strncpy(pass_raw, pass.c_str(), pass.length() + 1);
        ui.enableAuth(login_raw, pass_raw);
    }

    all_chime_names_csv.reserve(all_chime_list.size() * 32);
    for(int i = 0; i < all_chime_list.size(); i++) {
        if(i > 0) all_chime_names_csv += ",";
        size_t loc = std::string::npos;
        std::string tmp = std::string(all_chime_list[i]->get_long_title());
        std::replace(tmp.begin(), tmp.end(), ',', '/');
        all_chime_names_csv += tmp;
    }

    // Due to a bug in GyverPortal, handle uploads on our own
    ui.server.on("/prefs_restore", HTTP_POST, []() {
            ui.server.send(200, "text/html", F("<script>setInterval(function(){if(history.length>0)window.history.back();else window.location.href='/';},500);</script>"));
        }, []() {
            HTTPUpload& u = ui.server.upload();
            switch(u.status) {
                case UPLOAD_FILE_START:
                    ESP_LOGI(LOG_TAG, "Upload started, expect %i bytes", ui.server.clientContentLength());
                    if(begin_settings_write()) {
                        prefs_uploading = true;
                    }
                    break;

                case UPLOAD_FILE_WRITE:
                    ESP_LOGI(LOG_TAG, "Received %i bytes", u.currentSize);
                    if(prefs_uploading) {
                        write_settings_chunk((const char*) u.buf, u.currentSize);
                    }
                    break;

                case UPLOAD_FILE_ABORTED:
                    ESP_LOGI(LOG_TAG, "Upload aborted");
                    if(prefs_uploading) {
                        end_settings_write();
                        change_state(STATE_RESTART, TRANSITION_NONE);
                    }
                    break;

                case UPLOAD_FILE_END:
                    ESP_LOGI(LOG_TAG, "End: Received %i bytes", u.totalSize);
                    if(prefs_uploading) {
                        end_settings_write();
                        change_state(STATE_RESTART, TRANSITION_NONE);
                    }
                    break;
            }
        }
    );
    ui.start();

    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        AdminTaskFunction,
        "ADM",
        6000,
        nullptr,
        pisosTASK_PRIORITY_WEBADMIN,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}