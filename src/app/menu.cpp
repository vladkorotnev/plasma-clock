#include <app/menu.h>
#include <service/prefs.h>
#include <service/httpfvu.h>
#include <state.h>
#include <views/menu/menu.h>
#include <network/netmgr.h>
#include <rsrc/common_icons.h>
#include <service/time.h>
#include <service/localize.h>
#include <LittleFS.h>

const char LOG_TAG[] = "MENU";

class UptimeView: public MenuInfoItemView {
public:
    UptimeView(): MenuInfoItemView(localized_string("Uptime"), "") {
        bottom_label->stopped = true;
    }

    void step() {
        tk_time_of_day_t uptime = get_uptime();
        snprintf(buf, 16, "%02d:%02d:%02d", uptime.hour, uptime.minute, uptime.second, uptime.millisecond);
        bottom_label->set_string(buf);
        MenuInfoItemView::step();
    }

private:
    char buf[16];
};

class DiskSpaceView: public MenuInfoItemView {
public:
    DiskSpaceView(): MenuInfoItemView(localized_string("Disk Space"), "") {
    }

    void prepare() override {
        if(buf[0] == 0) {
            // Having this as a static item view causes a deadlock on boot, so we need to set the value in runtime
            snprintf(buf, 31, "%.02dK (%.02dK %s)", LittleFS.totalBytes() / 1024, (LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024, localized_string("free"));
            bottom_label->set_string(buf);
        }
        MenuInfoItemView::prepare();
    }

private:
    char buf[32] = { 0 };
};

#if HAS(LIGHT_SENSOR) && HAS(VARYING_BRIGHTNESS)
class LightSensorCalibrationView: public Composite {
public:
    LightSensorCalibrationView(const char * title, prefs_key_t key, AmbientLightSensor * lightSensor) {
        top_label = new StringScroll(find_font(FONT_STYLE_UI_TEXT), title);
        top_label->start_at_visible = true;
        top_label->holdoff = 100;
        add_composable(top_label);
        sensor = lightSensor;
        prefs_key = key;
        wants_clear_surface = true;
    }

    void prepare() override {
        set_value = prefs_get_int(prefs_key);
        Composite::prepare();
    }

    void render(FantaManipulator *fb) override {
        Composite::render(fb);

        int steps_per_division = (max_value - min_value) / (fb->get_width() - 3);
        int set_val_px = (set_value - min_value) / steps_per_division;
        int cur_val_px = (cur_value - min_value) / steps_per_division;
        
        // Backdrop
        fb->rect(0, 9, fb->get_width() - 2, 14, false);

        // Filled line for the pointer
        fb->line(cur_val_px + 1, 9, cur_val_px + 1, 14);

        // Two dots on the outline for the current setting
        fb->plot_pixel(set_val_px + 1, 8, true);
        fb->plot_pixel(set_val_px + 1, 15, true);
        // and a blinking line
        if(cursor_visible) {
            fb->line(set_val_px + 1, 9, set_val_px + 1, 14);
        }
    }

    void step() {
        cur_value = sensor->read();

        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            set_value = cur_value;
            prefs_set_int(prefs_key, set_value);
        }

        cursor_counter ++;
        if(cursor_counter == 30) {
            cursor_visible ^= 1;
            cursor_counter = 0;
        }
    }

private:
    AmbientLightSensor * sensor = nullptr;
    StringScroll * top_label = nullptr;
    prefs_key_t prefs_key = nullptr;
    int cur_value = 0;
    int min_value = 0;
    int max_value = 4096;
    int set_value = 0;
    bool cursor_visible = false;
    uint8_t cursor_counter = 0;
};
#endif

AppShimMenu::AppShimMenu(Beeper *b, NewSequencer *s, Yukkuri *y, AmbientLightSensor *als): ProtoShimNavMenu::ProtoShimNavMenu() {
    beeper = b;
    yukkuri = y;
    std::function<void(bool, Renderable*)> normalActivationFunction = [this](bool isActive, Renderable* instance) {
        if(isActive) push_renderable(instance, TRANSITION_NONE);
        else pop_renderable(TRANSITION_NONE);
    };

    // NB: This leaks a lot of objects on purpose.
    // This object is not supposed to be instantiated more than once in the firmware.

    static MenuTimeSettingView * ts_view = nullptr;
    static MenuDateSettingView * ds_view = nullptr;
    static ListView * clock_menu = new ListView();
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("24-hour display"), PREFS_KEY_DISP_24_HRS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Show seconds"), PREFS_KEY_SHOW_SECONDS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Blink dots"), PREFS_KEY_BLINK_SEPARATORS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Tick sound"), PREFS_KEY_TICKING_SOUND));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Ticking only when screen on"), PREFS_KEY_NO_SOUND_WHEN_OFF));

        // Possible chime values
        // 0: No chime
        // 1: Only chime
        // 2: Only beep
        // 3: Beep + chime
        // 4: Talk
        // 5: Talk + chime
        // 6: Talk + beep
        // 7: Talk + beep + chime
        // thus, bit allocations are: 0bxxxxxTBC
    
    const int CHIME_MODE_CHIME_BIT_NO = 0;
    const int CHIME_MODE_BEEP_BIT_NO = 1;
    const int CHIME_MODE_TALK_BIT_NO = 2;

    clock_menu->add_view(new MenuListSelectorView(localized_string("Hourly chime"), {
        localized_string("No chime"),
        localized_string("Melody chime"),
        localized_string("6 second beep signal"),
        localized_string("6 beeps → melody"),
#if HAS(AQUESTALK)
        localized_string("Talking clock"),
        localized_string("Talk → Melody"),
        localized_string("6 beeps → Talk"),
        localized_string("6 beeps → Talk → Melody")
#endif
    }, 
        (prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON) ? (1 << CHIME_MODE_CHIME_BIT_NO) : 0) | 
        (prefs_get_bool(PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL) ? (1 << CHIME_MODE_BEEP_BIT_NO) : 0) | 
        (prefs_get_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR) ? (1 << CHIME_MODE_TALK_BIT_NO) : 0),
    normalActivationFunction, [](int newChimeValue) {
        prefs_set_bool(PREFS_KEY_HOURLY_CHIME_ON, (newChimeValue & (1 << CHIME_MODE_CHIME_BIT_NO)) != 0);
        prefs_set_bool(PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL, (newChimeValue & (1 << CHIME_MODE_BEEP_BIT_NO)) != 0);
        prefs_set_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR, (newChimeValue & (1 << CHIME_MODE_TALK_BIT_NO)) != 0);

        ESP_LOGI(LOG_TAG, "New chime state: hourly chime = %i, precise time signal = %i, voice announce = %i", prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON), prefs_get_bool(PREFS_KEY_HOURLY_PRECISE_TIME_SIGNAL), prefs_get_bool(PREFS_KEY_VOICE_ANNOUNCE_HOUR));
    }));

    clock_menu->add_view(new MenuMelodySelectorPreferenceView(s, localized_string("First chime melody"), PREFS_KEY_FIRST_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuMelodySelectorPreferenceView(s, localized_string("Other chimes melody"), PREFS_KEY_HOURLY_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Chime from"), PREFS_KEY_HOURLY_CHIME_START_HOUR, 0, 23, 1, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Chime until"), PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 0, 23, 1, normalActivationFunction));
#if HAS(AQUESTALK)

    std::function<void(int)> yukkuriTestFunction = [y](int) {
        if(y->is_speaking()) {
            y->cancel_current();
        }

        const char * test_utterance;
        switch(active_tts_language()) {
            case TTS_LANG_JA:
                test_utterance = ".yukkuri_siteittene?";
                break;

            case TTS_LANG_RU:
                test_utterance = ".pya'_tunitsa- vu;efi'rye;kapita'ru/sho'u;po'rye/chujye'_su pie'rubaya;turo'ika;iguroko'fu,fusutu'-jiyu-.";
                break;

            case TTS_LANG_EN:
            default:
                test_utterance = ".a'iru/he'bu;ei;nanba'-;nain ei;nanba'-;si_ku_su/uizu;e'ku_su_tsura/so'-_su tuu/nanba'-;fo'-ti/fai_fu_su,ennda;ra'-ji;so'-da.";
                break;
        }

        y->speak(test_utterance);
    };

    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Speak on headpat"), PREFS_KEY_VOICE_SPEAK_ON_HEADPAT));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("24-hour announcements"), PREFS_KEY_VOICE_24_HRS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Speak date on first chime"), PREFS_KEY_VOICE_ANNOUNCE_DATE));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Voice speed"), PREFS_KEY_VOICE_SPEED, 10, 200, 1, normalActivationFunction, yukkuriTestFunction));
    clock_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Voice language"), 
        {localized_string("English"), localized_string("Russian"), localized_string("Japanese")},
        PREFS_KEY_TTS_LANGUAGE,
        normalActivationFunction,
        yukkuriTestFunction
    ));
    clock_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Voice mode"), 
        {localized_string("Clear"), localized_string("Loud")},
        PREFS_KEY_VOICE_MODE_RESAMPLING,
        normalActivationFunction,
        yukkuriTestFunction
    ));
#endif
    clock_menu->add_view(new MenuActionItemView(localized_string("Set time"), [this]() {
        tk_time_of_day_t now = get_current_time_coarse();
        if(ts_view != nullptr) delete ts_view;
        ts_view = new MenuTimeSettingView(beeper, now.hour, now.minute, [this](int h, int m, int s) {
            tk_time_of_day_t updated_time = {
                .hour = h, .minute = m, .second = 0, .millisecond = 0
            };
            set_current_time(updated_time);
            pop_renderable(TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        });
        push_renderable(ts_view, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }));
    clock_menu->add_view(new MenuActionItemView(localized_string("Set date"), [this]() {
        tk_date_t today = get_current_date();
        if(ds_view != nullptr) delete ds_view;
        ds_view = new MenuDateSettingView(beeper, today.year, today.month, today.day, [this](int y, int m, int d) {
            tk_date_t new_date = {
                .year = y, .month = m, .day = d
            };
            set_current_date(new_date);
            pop_renderable(TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        });
        push_renderable(ds_view, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Use internet time"), PREFS_KEY_TIMESERVER_ENABLE));
    clock_menu->add_view(new MenuInfoItemView(localized_string("NTP/Timezone"), localized_string("Please use the Web UI to configure.")));

    static ListView * display_menu = new ListView();
    static ListView * screen_times = new ListView();
    display_menu->add_view(new MenuActionItemView(localized_string("Screen Times"), [this](){ push_submenu(screen_times); }));
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Clock"), PREFS_KEY_SCRN_TIME_CLOCK_SECONDS, 0, 3600, 1, normalActivationFunction));
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Alarm countdown"), PREFS_KEY_SCRN_TIME_NEXT_ALARM_SECONDS, 0, 3600, 1, normalActivationFunction));
#if HAS(TEMP_SENSOR)
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Thermometer"), PREFS_KEY_SCRN_TIME_INDOOR_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Switchbot Meter"), PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Current Weather"), PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS, 0, 3600, 1, normalActivationFunction));
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("2-day Forecast"), PREFS_KEY_SCRN_TIME_FORECAST_SECONDS, 0, 3600, 1, normalActivationFunction));
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Hourly Precipitation % Graph"), PREFS_KEY_SCRN_TIME_PRECIPITATION_SECONDS, 0, 3600, 1, normalActivationFunction));
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Hourly Pressure Graph"), PREFS_KEY_SCRN_TIME_PRESSURE_SECONDS, 0, 3600, 1, normalActivationFunction));
#if HAS(WORDNIK_API)
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Wordnik"), PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
        screen_times->add_view(new MenuNumberSelectorPreferenceView(localized_string("Foobar2000"), PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS, 0, 3600, 1, normalActivationFunction));
#if HAS(VARYING_BRIGHTNESS)
    display_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Brightness"),
        {
            localized_string("Dim"),
            localized_string("Bright"),
#if HAS(LIGHT_SENSOR)
            localized_string("Automatic"),
#endif
        },
        PREFS_KEY_BRIGHTNESS_MODE,
        normalActivationFunction
    ));
#endif
#if HAS(DISPLAY_BLANKING)
    display_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Blank display after (s)"), PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS, 0, 21600, 1, normalActivationFunction));
#endif
    display_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Turn display off after (s)"), PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS, 0, 72000, 1, normalActivationFunction));
#if HAS(MOTION_SENSOR)
    display_menu->add_view(new MenuBooleanSettingView(localized_string("Ignore motion sensor by schedule"), PREFS_KEY_IGNORE_MOTION_SCHEDULE_ON));
    display_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Start ignore at hour"), PREFS_KEY_IGNORE_MOTION_START_HR, 0, 23, 1, normalActivationFunction));
    display_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("End ignore at hour"), PREFS_KEY_IGNORE_MOTION_END_HR, 0, 23, 1, normalActivationFunction));
#endif
    display_menu->add_view(new MenuBooleanSettingView(localized_string("Keypress beep"), PREFS_KEY_BUTTON_BEEP, [b](bool newValue) {
        hid_set_key_beeper(newValue ? b : nullptr);
    }));
    display_menu->add_view(new MenuBooleanSettingView(localized_string("Use Fahrenheit for temperature"), PREFS_KEY_WEATHER_USE_FAHRENHEIT));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Transition"), 
        {localized_string("Off"), localized_string("Wipe"), localized_string("Slide Left"), localized_string("Slide Right"), localized_string("Slide Up"), localized_string("Slide Down"), localized_string("(Randomize)")},
        PREFS_KEY_TRANSITION_TYPE,
        normalActivationFunction
    ));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Scroll Speed"), 
        {localized_string("Slow"), localized_string("Medium"), localized_string("Fast"), localized_string("Sonic")},
        PREFS_KEY_DISP_SCROLL_SPEED,
        normalActivationFunction
    ));
    display_menu->add_view(new MenuBooleanSettingView(localized_string("FPS counter"), PREFS_KEY_FPS_COUNTER));
    display_menu->add_view(new MenuBooleanSettingView(localized_string("Weather effects"), PREFS_KEY_WEATHER_OVERLAY));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("WiFi signal"),
        {localized_string("Off"), localized_string("Disconnected"), localized_string("Display power on"), localized_string("Always")},
        PREFS_KEY_WIFI_ICON,
        normalActivationFunction
    ));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        "Language / Язык", 
        {"English", "Русский", "日本語"},
        PREFS_KEY_DISP_LANGUAGE,
        normalActivationFunction
    ));

#if (HAS(TEMP_SENSOR) || (HAS(LIGHT_SENSOR) && HAS(VARYING_BRIGHTNESS)))
    static ListView * calibration_menu = new ListView();
    #if HAS(TEMP_SENSOR)
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Temperature (\u00B0C)"), PREFS_KEY_TEMP_SENSOR_TEMP_OFFSET, -50, 50, 1, normalActivationFunction));
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Humidity"), PREFS_KEY_TEMP_SENSOR_HUM_OFFSET, -50, 50, 1, normalActivationFunction));
    #endif
    #if HAS(LIGHT_SENSOR) && HAS(VARYING_BRIGHTNESS)
    calibration_menu->add_view(new LightSensorCalibrationView(
        localized_string("Display dimming threshold"), PREFS_KEY_LIGHTNESS_THRESH_DOWN, als
    ));
    calibration_menu->add_view(new LightSensorCalibrationView(
        localized_string("Display brightening threshold"), PREFS_KEY_LIGHTNESS_THRESH_UP, als
    ));
    #endif
#endif

    static ListView * system_info = new ListView();
    system_info->add_view(new MenuInfoItemView(localized_string("OS Name"), PRODUCT_NAME));
    system_info->add_view(new MenuInfoItemView(localized_string("OS Type"), FVU_FLAVOR));
    system_info->add_view(new MenuInfoItemView(localized_string("OS Version"), PRODUCT_VERSION));
    system_info->add_view(new MenuInfoItemView(localized_string("OS Build"), get_current_version_info().fs_current));
    system_info->add_view(new MenuInfoItemView(localized_string("WiFi Name"), NetworkManager::network_name()));
    static char buf_ip[17] = { 0 };
    String tmp = NetworkManager::current_ip();
    strncpy(buf_ip, tmp.c_str(), 16);
    static char buf_mac[20] = { 0 };
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    snprintf(buf_mac, 19, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    system_info->add_view(new MenuInfoItemView(localized_string("WiFi IP"), buf_ip));
    system_info->add_view(new MenuInfoItemView(localized_string("MAC Address"), buf_mac));
    system_info->add_view(new DiskSpaceView());
    system_info->add_view(new UptimeView());

    static ListView * service_settings = new ListView();
    service_settings->add_view(new MenuBooleanSettingView(localized_string("Remote Control Server"), PREFS_KEY_REMOTE_SERVER));
#if HAS(SERIAL_MIDI)
    service_settings->add_view(new MenuBooleanSettingView(localized_string("Serial MIDI Input"), PREFS_KEY_SERIAL_MIDI));
#endif
#if HAS(HTTPFVU)
    service_settings->add_view(new MenuBooleanSettingView(localized_string("Periodically check for updates"), PREFS_KEY_FVU_AUTO_CHECK));
    service_settings->add_view(new MenuNumberSelectorPreferenceView(localized_string("Check interval (minutes)"), PREFS_KEY_FVU_AUTO_CHECK_INTERVAL_MINUTES, 1, 60 * 24, 1, normalActivationFunction));
    service_settings->add_view(new MenuBooleanSettingView(localized_string("Automatically download and install updates"), PREFS_KEY_FVU_AUTO_INSTALL));
    service_settings->add_view(new MenuActionItemView(localized_string("Check for updates now"), []() {
        push_state(STATE_HTTPFVU, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }));
#endif
    service_settings->add_view(new MenuActionItemView(localized_string("Reset Webadmin Password"), []() {
        prefs_set_string(PREFS_KEY_ADMIN_PASS, "");
        change_state(STATE_RESTART, TRANSITION_NONE);
    }, nullptr, localized_string("Press \u001A")));

    static const uint8_t status_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf0, 0x11, 0xf8, 0x3e, 0xfc, 0x7b, 0xc2, 0x7b, 0xfe, 0x7b, 0xce, 0x7f, 0xce, 
        0x7f, 0xfe, 0x6e, 0x3e, 0x57, 0xfe, 0x50, 0x3e, 0x3f, 0xdc, 0x7c, 0x38, 0x7d, 0xf0, 0x3c, 0x00
    };

    static const sprite_t status_icns = { .width = 16, .height = 16, .data = status_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

    static const uint8_t clock_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf8, 0x18, 0x0c, 0x37, 0x76, 0x6f, 0xfb, 0x5f, 0x7d, 0x5f, 0x7d, 0x5f, 0x7d, 
        0x4f, 0x09, 0x5f, 0xfd, 0x5f, 0xfd, 0x5f, 0xfd, 0x6f, 0xfb, 0x37, 0x76, 0x18, 0x0c, 0x0f, 0xf8
    };

    static const sprite_t clock_icns = { .width = 16, .height = 16, .data = clock_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

    static const uint8_t display_icns_data[] = {
        // By PiiXL
        0x00, 0x70, 0x03, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x60, 0x19, 0x4c, 0x09, 0x50, 0x09, 
        0x40, 0x09, 0x40, 0x0f, 0x40, 0x09, 0x40, 0x0f, 0x60, 0x19, 0x7f, 0xff, 0x00, 0x00, 0x18, 0x0c
    };

    static const sprite_t display_icns = { .width = 16, .height = 16, .data = display_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

    static const uint8_t wrench_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x03, 0xe0, 0x07, 0xc0, 0x0f, 0x80, 0x0f, 0x00, 0x0f, 0x00, 0x0f, 0x01, 0x0f, 0x83, 
        0x0f, 0xc7, 0x1f, 0xff, 0x3f, 0xff, 0x6f, 0xfe, 0x5f, 0xfc, 0x3b, 0x00, 0x76, 0x00, 0x6c, 0x00
    };

    static const sprite_t wrench_icns = { .width = 16, .height = 16, .data = wrench_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

    static const uint8_t stopwatch_icns_data[] = {
        // By PiiXL
        0x07, 0xc0, 0x08, 0x20, 0x08, 0x20, 0x06, 0xc0, 0x00, 0x04, 0x37, 0xfa, 0x6c, 0x0c, 0xdb, 0x76, 
	    0xd7, 0x7a, 0xd7, 0x7a, 0xd7, 0x0a, 0xd7, 0xfa, 0xd7, 0xfa, 0xdb, 0xf6, 0x6c, 0x0c, 0x37, 0xf8
    };

    static const sprite_t stopwatch_icns = { .width = 16, .height = 16, .data = stopwatch_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

    static const uint8_t weather_icns_data[] = {
        // By PiiXL
        0x07, 0x00, 0x0f, 0x80, 0x0f, 0xb0, 0x77, 0x78, 0xfb, 0xf8, 0xff, 0xf6, 0xbf, 0xff, 0xcf, 0xff, 
        0x7f, 0xfe, 0x00, 0x00, 0x22, 0x24, 0x44, 0x48, 0x89, 0x10, 0x02, 0x00, 0x24, 0x20, 0x40, 0x40
    };

    static const sprite_t weather_icns = { .width = 16, .height = 16, .data = weather_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };

#if HAS(BALANCE_BOARD_INTEGRATION)
    static const uint8_t weight_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x08, 0x10, 0x28, 0x14, 0x28, 0x14, 0xab, 0xd5, 
	    0xab, 0xd5, 0xab, 0xd5, 0x28, 0x14, 0x28, 0x14, 0x08, 0x10, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00
    };

    static const sprite_t weight_icns = { .width = 16, .height = 16, .data = weight_icns_data, .mask = nullptr, .format = SPRFMT_HORIZONTAL };
#endif

    static ListView * settings_menu = new ListView();
    settings_menu->add_view(new MenuActionItemView(localized_string("Clock"), [this](){ push_submenu(clock_menu); }, &clock_icns));
    settings_menu->add_view(new MenuActionItemView(localized_string("Display"), [this](){ push_submenu(display_menu); }, &display_icns));
#if (HAS(TEMP_SENSOR) || (HAS(LIGHT_SENSOR) && HAS(VARYING_BRIGHTNESS)))
    settings_menu->add_view(new MenuActionItemView(localized_string("Offsets"), [this](){ push_submenu(calibration_menu); }, &icon_thermo_1616));
#endif
    settings_menu->add_view(new MenuActionItemView(localized_string("System"), [this](){ push_submenu(service_settings); }, &wrench_icns));
    settings_menu->add_view(new MenuActionItemView(localized_string("Status"), [this](){ push_submenu(system_info); scroll_guidance->right = false; }, &status_icns));
    settings_menu->add_view(new MenuInfoItemView(localized_string("Notice"), localized_string("FULL_SETTINGS_NOTICE")));
    settings_menu->add_view(new MenuActionItemView(localized_string("Save & Restart"), [this](){
        prefs_force_save();
        change_state(STATE_RESTART, TRANSITION_NONE);
    }, &good_icns));

    main_menu->add_view(new MenuActionItemView(localized_string("Clock"), [](){ pop_state(STATE_MENU, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &clock_icns));
    main_menu->add_view(new MenuActionItemView(localized_string("Timer"), [this](){ push_state(STATE_TIMER_EDITOR, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &hourglass_icns));
    main_menu->add_view(new MenuActionItemView(localized_string("Stopwatch"), [](){ push_state(STATE_STOPWATCH, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &stopwatch_icns));
    main_menu->add_view(new MenuActionItemView(localized_string("Weather"), []() { push_state(STATE_WEATHER, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &weather_icns));
    main_menu->add_view(new MenuActionItemView(localized_string("Alarm"), [](){ push_state(STATE_ALARM_EDITOR, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &alarm_icns));
    main_menu->add_view(new MenuActionItemView(localized_string("Music Box"), [](){ push_state(STATE_MUSICBOX, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &music_icns));
#if HAS(BALANCE_BOARD_INTEGRATION)
    main_menu->add_view(new MenuActionItemView(localized_string("Weighing"), [this](){ push_state(STATE_WEIGHING, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &weight_icns));
#endif
    main_menu->add_view(new MenuActionItemView(localized_string("Settings"), [this](){ push_submenu(settings_menu); }, &wrench_icns));
#if HAS(PIXEL_CAVE)
    main_menu->add_view(new MenuActionItemView("Pixel Cave", []() { push_state(STATE_PIXEL_CAVE, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &good_icns));
#endif
#if HAS(PLAYGROUND)
    main_menu->add_view(new MenuActionItemView("Test", []() { push_state(STATE_PLAYGROUND, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &good_icns));
#endif
}   

void AppShimMenu::prepare() {
    ProtoShimNavMenu::prepare();
    last_touch_time = xTaskGetTickCount();
}

void AppShimMenu::render(FantaManipulator *fb) {
    ProtoShimNavMenu::render(fb);
    if(fb->get_width() != last_width) {
        last_width = fb->get_width();
        last_touch_time = xTaskGetTickCount();
    }
}

void AppShimMenu::step() {
    ProtoShimNavMenu::step();
    if(hid_test_key_any()) {
        last_touch_time = xTaskGetTickCount();
    } else if (xTaskGetTickCount() - last_touch_time >= pdMS_TO_TICKS(30000)) {
        pop_state(STATE_MENU, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }
}

void AppShimMenu::pop_renderable(transition_type_t transition) {
    if(back_stack.size() == 0) {
        // back button goes back, who would have guessed
        pop_state(STATE_MENU, TRANSITION_SLIDE_HORIZONTAL_LEFT);
        return;
    }

    ProtoShimNavMenu::pop_renderable(transition);
}
