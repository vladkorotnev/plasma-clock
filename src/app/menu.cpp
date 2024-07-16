#include <app/menu.h>
#include <service/prefs.h>
#include <state.h>
#include <views/menu/menu.h>
#include <network/netmgr.h>
#include <rsrc/common_icons.h>
#include <service/time.h>

AppShimMenu::AppShimMenu(Beeper *b): ProtoShimNavMenu::ProtoShimNavMenu() {
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
    clock_menu->add_view(new MenuBooleanSettingView("Blink dots", PREFS_KEY_BLINK_SEPARATORS));
    clock_menu->add_view(new MenuBooleanSettingView("Tick sound", PREFS_KEY_TICKING_SOUND));
    clock_menu->add_view(new MenuBooleanSettingView("Ticking only when screen on", PREFS_KEY_NO_SOUND_WHEN_OFF));
    clock_menu->add_view(new MenuBooleanSettingView("Hourly chime", PREFS_KEY_HOURLY_CHIME_ON));
    clock_menu->add_view(new MenuMelodySelectorPreferenceView(beeper, "First chime", PREFS_KEY_FIRST_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuMelodySelectorPreferenceView(beeper, "Other chimes", PREFS_KEY_HOURLY_CHIME_MELODY, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView("Chime from", PREFS_KEY_HOURLY_CHIME_START_HOUR, 0, 23, 1, normalActivationFunction));
    clock_menu->add_view(new MenuNumberSelectorPreferenceView("Chime until", PREFS_KEY_HOURLY_CHIME_STOP_HOUR, 0, 23, 1, normalActivationFunction));
    clock_menu->add_view(new MenuActionItemView("Set time", [this]() {
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
    clock_menu->add_view(new MenuActionItemView("Set date", [this]() {
        tk_date_t today = get_current_date();
        if(ds_view != nullptr) delete ds_view;
        ds_view = new MenuDateSettingView(beeper, today.year, today.month, today.day, [this](int y, int m, int d) {
            // todo: set date
            tk_date_t new_date = {
                .year = y, .month = m, .day = d
            };
            set_current_date(new_date);
            pop_renderable(TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        });
        push_renderable(ds_view, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }));
    clock_menu->add_view(new MenuBooleanSettingView("Use internet time", PREFS_KEY_TIMESERVER_ENABLE));
    clock_menu->add_view(new MenuInfoItemView("NTP/Timezone", "Please use the Web UI to configure."));

    static ListView * display_menu = new ListView();
    static ListView * screen_times = new ListView();
    display_menu->add_view(new MenuActionItemView("Screen Times", [this](){ push_submenu(screen_times); }));
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Clock", PREFS_KEY_SCRN_TIME_CLOCK_SECONDS, 0, 3600, 1, normalActivationFunction));
#if HAS(TEMP_SENSOR)
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Thermometer", PREFS_KEY_SCRN_TIME_INDOOR_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
#if HAS(SWITCHBOT_METER_INTEGRATION)
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Switchbot Meter", PREFS_KEY_SCRN_TIME_REMOTE_WEATHER_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Weather", PREFS_KEY_SCRN_TIME_OUTDOOR_SECONDS, 0, 3600, 1, normalActivationFunction));
#if HAS(WORDNIK_API)
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Wordnik", PREFS_KEY_SCRN_TIME_WORD_OF_THE_DAY_SECONDS, 0, 3600, 1, normalActivationFunction));
#endif
        screen_times->add_view(new MenuNumberSelectorPreferenceView("Foobar2000", PREFS_KEY_SCRN_TIME_FOOBAR_SECONDS, 0, 3600, 1, normalActivationFunction));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        "Transition", 
        {"(Off)", "Wipe", "Slide Left", "Slide Right", "Slide Up", "Slide Down", "(Randomize)"},
        PREFS_KEY_TRANSITION_TYPE,
        normalActivationFunction
    ));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        "Scroll Speed", 
        {"Slow", "Medium", "Fast", "Sonic"},
        PREFS_KEY_DISP_SCROLL_SPEED,
        normalActivationFunction
    ));
    display_menu->add_view(new MenuBooleanSettingView("FPS counter", PREFS_KEY_FPS_COUNTER));
    display_menu->add_view(new MenuListSelectorPreferenceView(
        "WiFi signal",
        {"Off", "Disconnected", "Display power on", "Always"},
        PREFS_KEY_WIFI_ICON,
        normalActivationFunction
    ));

    static ListView * calibration_menu = new ListView();
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView("Temperature", PREFS_KEY_TEMP_SENSOR_TEMP_OFFSET, -50, 50, 1, normalActivationFunction));
    calibration_menu->add_view(new MenuNumberSelectorPreferenceView("Humidity", PREFS_KEY_TEMP_SENSOR_HUM_OFFSET, -50, 50, 1, normalActivationFunction));

    static ListView * system_info = new ListView();
    system_info->add_view(new MenuInfoItemView("OS Type", PRODUCT_NAME));
    system_info->add_view(new MenuInfoItemView("OS Version", PRODUCT_VERSION));
    system_info->add_view(new MenuInfoItemView("WiFi Name", NetworkManager::network_name()));
    static char buf_ip[17] = { 0 };
    String tmp = NetworkManager::current_ip();
    strncpy(buf_ip, tmp.c_str(), 16);
    system_info->add_view(new MenuInfoItemView("WiFi IP", buf_ip));

    static const uint8_t status_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf0, 0x11, 0xf8, 0x3e, 0xfc, 0x7b, 0xc2, 0x7b, 0xfe, 0x7b, 0xce, 0x7f, 0xce, 
        0x7f, 0xfe, 0x6e, 0x3e, 0x57, 0xfe, 0x50, 0x3e, 0x3f, 0xdc, 0x7c, 0x38, 0x7d, 0xf0, 0x3c, 0x00
    };

    static const sprite_t status_icns = { .width = 16, .height = 16, .data = status_icns_data, .mask = nullptr };

    static const uint8_t clock_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x0f, 0xf8, 0x18, 0x0c, 0x37, 0x76, 0x6f, 0xfb, 0x5f, 0x7d, 0x5f, 0x7d, 0x5f, 0x7d, 
        0x4f, 0x09, 0x5f, 0xfd, 0x5f, 0xfd, 0x5f, 0xfd, 0x6f, 0xfb, 0x37, 0x76, 0x18, 0x0c, 0x0f, 0xf8
    };

    static const sprite_t clock_icns = { .width = 16, .height = 16, .data = clock_icns_data, .mask = nullptr };

    static const uint8_t display_icns_data[] = {
        // By PiiXL
        0x00, 0x70, 0x03, 0x80, 0x1c, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x60, 0x19, 0x4c, 0x09, 0x50, 0x09, 
        0x40, 0x09, 0x40, 0x0f, 0x40, 0x09, 0x40, 0x0f, 0x60, 0x19, 0x7f, 0xff, 0x00, 0x00, 0x18, 0x0c
    };

    static const sprite_t display_icns = { .width = 16, .height = 16, .data = display_icns_data, .mask = nullptr };

    static const uint8_t wrench_icns_data[] = {
        // By PiiXL
        0x00, 0x00, 0x03, 0xe0, 0x07, 0xc0, 0x0f, 0x80, 0x0f, 0x00, 0x0f, 0x00, 0x0f, 0x01, 0x0f, 0x83, 
        0x0f, 0xc7, 0x1f, 0xff, 0x3f, 0xff, 0x6f, 0xfe, 0x5f, 0xfc, 0x3b, 0x00, 0x76, 0x00, 0x6c, 0x00
    };

    static const sprite_t wrench_icns = { .width = 16, .height = 16, .data = wrench_icns_data, .mask = nullptr };

    static ListView * settings_menu = new ListView();
    settings_menu->add_view(new MenuActionItemView("Clock", [this](){ push_submenu(clock_menu); }, &clock_icns));
    settings_menu->add_view(new MenuActionItemView("Display", [this](){ push_submenu(display_menu); }, &display_icns));
    settings_menu->add_view(new MenuActionItemView("Offsets", [this](){ push_submenu(calibration_menu); }, &icon_thermo_1616));
    settings_menu->add_view(new MenuActionItemView("Status", [this](){ push_submenu(system_info); scroll_guidance->right = false; }, &status_icns));
    settings_menu->add_view(new MenuInfoItemView("Notice", "Full settings are only available in the Web UI"));
    settings_menu->add_view(new MenuActionItemView("Save & Restart", [this](){ 
        prefs_force_save();
        BeepSequencer * s = new BeepSequencer(beeper);
        s->play_sequence(tulula_fvu, CHANNEL_ALARM, SEQUENCER_NO_REPEAT);
        s->wait_end_play();
        ESP.restart();
    }, &good_icns));

    main_menu->add_view(new MenuActionItemView("Timer", [this](){ }, &hourglass_icns));
    main_menu->add_view(new MenuActionItemView("Alarm", [this](){ push_state(STATE_ALARM_EDITOR, TRANSITION_SLIDE_HORIZONTAL_LEFT); }, &alarm_icns));
    main_menu->add_view(new MenuActionItemView("Settings", [this](){ push_submenu(settings_menu); }, &wrench_icns));
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