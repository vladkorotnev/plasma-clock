#include <app/menu.h>
#include <service/prefs.h>
#include <state.h>
#include <views/menu/menu.h>
#include <network/netmgr.h>
#include <rsrc/common_icons.h>
#include <service/time.h>
#include <service/localize.h>

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

AppShimMenu::AppShimMenu(Beeper *b, NewSequencer *s): ProtoShimNavMenu::ProtoShimNavMenu() {
    beeper = b;
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
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Blink dots"), PREFS_KEY_BLINK_SEPARATORS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Tick sound"), PREFS_KEY_TICKING_SOUND));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Ticking only when screen on"), PREFS_KEY_NO_SOUND_WHEN_OFF));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Hourly chime"), PREFS_KEY_HOURLY_CHIME_ON));
    clock_menu->add_view(new MenuMelodySelectorPreferenceView(s, localized_string("First chime"), PREFS_KEY_FIRST_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuMelodySelectorPreferenceView(s, localized_string("Other chimes"), PREFS_KEY_HOURLY_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Chime from"), PREFS_KEY_HOURLY_CHIME_START_HOUR, 0, 23, 1, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Chime until"), PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 0, 23, 1, normalActivationFunction));
#if HAS(AQUESTALK)
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Speak hour"), PREFS_KEY_VOICE_ANNOUNCE_HOUR));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("24-hour announcements"), PREFS_KEY_DISP_24_HRS));
    clock_menu->add_view(new MenuBooleanSettingView(localized_string("Speak date on first chime"), PREFS_KEY_VOICE_ANNOUNCE_DATE));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Voice speed"), PREFS_KEY_VOICE_SPEED, 10, 200, 1, normalActivationFunction));
    clock_menu->add_view(new MenuListSelectorPreferenceView(
        localized_string("Voice language"), 
        {localized_string("English"), localized_string("Russian"), localized_string("Japanese")},
        PREFS_KEY_TTS_LANGUAGE,
        normalActivationFunction
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
        {"English", "Русский"},
        PREFS_KEY_DISP_LANGUAGE,
        normalActivationFunction
    ));

    static ListView * calibration_menu = new ListView();
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Temperature (\370C)"), PREFS_KEY_TEMP_SENSOR_TEMP_OFFSET, -50, 50, 1, normalActivationFunction));
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView(localized_string("Humidity"), PREFS_KEY_TEMP_SENSOR_HUM_OFFSET, -50, 50, 1, normalActivationFunction));

    static ListView * system_info = new ListView();
    system_info->add_view(new MenuInfoItemView(localized_string("OS Type"), PRODUCT_NAME));
    system_info->add_view(new MenuInfoItemView(localized_string("OS Version"), PRODUCT_VERSION));
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
    system_info->add_view(new UptimeView());
    system_info->add_view(new MenuBooleanSettingView(localized_string("Remote Control Server"), PREFS_KEY_REMOTE_SERVER));
#if HAS(SERIAL_MIDI)
    system_info->add_view(new MenuBooleanSettingView(localized_string("Serial MIDI Input"), PREFS_KEY_SERIAL_MIDI));
#endif

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
    settings_menu->add_view(new MenuActionItemView(localized_string("Offsets"), [this](){ push_submenu(calibration_menu); }, &icon_thermo_1616));
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
#if HAS(BALANCE_BOARD_INTEGRATION)
    main_menu->add_view(new MenuActionItemView(localized_string("Weighing"), [this](){ push_state(STATE_WEIGHING, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &weight_icns));
#endif
    main_menu->add_view(new MenuActionItemView(localized_string("Settings"), [this](){ push_submenu(settings_menu); }, &wrench_icns));
#if HAS(PLAYGROUND)
    main_menu->add_view(new MenuActionItemView("Test", []() { push_state(STATE_PLAYGROUND, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &good_icns));
#endif
}   

void AppShimMenu::prepare() {
    ProtoShimNavMenu::prepare();
    last_touch_time = xTaskGetTickCount();
}


void AppShimMenu::step() {
    ProtoShimNavMenu::step();
    if(hid_test_key_any()) {
        last_touch_time = xTaskGetTickCount();
    } else if (xTaskGetTickCount() - last_touch_time >= pdMS_TO_TICKS(back_stack.size() == 0 ? 5000 : 30000)) {
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