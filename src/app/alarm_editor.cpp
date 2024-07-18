#include <app/alarm_editor.h>
#include <views/menu/menu.h>
#include <state.h>
static const char day_letters[14] = {
    'S', 0,
    'M', 0,
    'T', 0,
    'W', 0,
    'T', 0,
    'F', 0,
    'S', 0,
};

class AlarmItemView: public Renderable {
public:
    AlarmItemView(int index, std::function<void(int)> onSelect):
        index(index),
        index_str {0},
        time_buf {0},
        setting {0},
        _action(onSelect)
    {
        itoa(index + 1, index_str, 10);
    }

    void prepare() {
        const alarm_setting_t * settings = get_alarm_list();
        setting = settings[index];
    }

    void render(FantaManipulator *fb) {
        // box around number
        fb->rect(0, 2, 10 + setting.enabled, 12, setting.enabled);
        fb->put_string(&keyrus0808_font, index_str, 2, 4, setting.enabled);

        fb->put_string(&keyrus0808_font, time_buf, 14, 0);

        if(setting.days == ALARM_DAY_GLOBAL_ENABLE || setting.days == 0) {
            fb->put_string(&keyrus0808_font, "Only Once", 14, 8);
        } else {
            for(int d = 0; d < 7; d++) {
                fb->put_string(&keyrus0808_font, &day_letters[d * 2], 14 + d * (keyrus0808_font.width + 1), 8, ALARM_ON_DAY(setting, d));
            }
        }
    }

    void step() {
        if(!setting.smart || setting.smart_margin_minutes == 0) {
            snprintf(time_buf, 16, "%02d:%02d", setting.hour, setting.minute);
        } else {
            tk_time_of_day_t alarm_time = { .hour = setting.hour, .minute = setting.minute, .second = 0, .millisecond = 0 };
            tk_time_of_day_t margin = { .hour = 0, .minute = setting.smart_margin_minutes, .second = 0, .millisecond = 0 };
            tk_time_of_day_t early_time = alarm_time - margin;
            snprintf(time_buf, 16, "%02d:%02d-%02d:%02d", early_time.hour, early_time.minute, setting.hour, setting.minute);
        }

        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            _action(index);
        }
    }

private:
    std::function<void(int)> _action;
    alarm_setting_t setting;
    int index;
    char index_str[4];
    char time_buf[16];
};

class AlarmDaySelectorView: public Composite {
public:
    AlarmDaySelectorView(const char * title, alarm_setting_t * setting, std::function<void(bool, Renderable*)> onActivated):
        isActive {false},
        label(new StringScroll(&keyrus0808_font, title)),
        setting(setting),
        cursor { 0 },
        onActivated(onActivated) {
            add_subrenderable(label);
        }
    
    ~AlarmDaySelectorView() {
        delete label;
    }

    void step() {
        Composite::step();

        if(isActive) {
            if(hid_test_key_state_repetition(KEY_DOWN) == KEYSTATE_HIT) {
                cursor++;
                if(cursor == 7) cursor = 0;
            } else if(hid_test_key_state_repetition(KEY_UP) == KEYSTATE_HIT) {
                cursor--;
                if(cursor < 0) cursor = 6;
            } else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
                uint8_t day_mask = ALARM_DAY_OF_WEEK(cursor);
                if((setting->days & day_mask) != 0) {
                    setting->days &= ~day_mask;
                } else {
                    setting->days |= day_mask;
                }
            } else if(hid_test_key_state(KEY_LEFT)) {
                onActivated(false, this);
                isActive = false;
            }
        } else {
            if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
                onActivated(true, this);
                isActive = true;
            }
        }
    }

    void render(FantaManipulator *fb) {
        Composite::render(fb);
        framecount++;
        if(framecount == 30) {
            cursorShows = !cursorShows;
            framecount = 0;
        }

        if(isActive || setting->days != 0) {
            for(int d = 0; d < 7; d++) {
                int ltr_x = fb->get_width() - 70 + d * (keyrus0808_font.width + 2);
                bool lit_up = (setting->days & ALARM_DAY_OF_WEEK(d)) != 0;

                if(isActive && d == cursor && cursorShows) {
                    fb->rect(ltr_x - 2, 7, ltr_x + 8, 15, lit_up);
                }
                fb->put_string(&keyrus0808_font, &day_letters[d * 2], ltr_x, 8, lit_up);
            }
        } else {
            static const char one_time[] = "Only Once";
            fb->put_string(&keyrus0808_font, one_time, fb->get_width() - measure_string_width(&keyrus0808_font, one_time), 8);
        }
    }

private:
    uint8_t framecount;
    bool isActive;
    bool cursorShows;
    StringScroll * label;
    alarm_setting_t * setting;
    int cursor;
    std::function<void(bool, Renderable*)> onActivated;
};

class AppShimAlarmEditor::AlarmEditorView: public ListView {
public:
    AlarmEditorView(alarm_setting_t *alm, Beeper *b, std::function<void(bool, Renderable*)> activation, std::function<void(bool, Renderable*)> pushPop): ListView::ListView() {
        setting = alm;
        ts_view = new MenuTimeSettingView(b, setting->hour, setting->minute, [this, pushPop](int h, int m, int s) {
            setting->hour = h;
            setting->minute = m;
            pushPop(false, ts_view);
        });

        add_view(new MenuBooleanSelectorView("Enabled", alm->enabled, [alm](bool newEnabled) {
            alm->enabled = newEnabled;
        }));

        add_view(new AlarmDaySelectorView("Days", alm, activation));
        add_view(new MenuActionItemView("Time", [pushPop, this]() { pushPop(true, ts_view); }, nullptr, time_str));
    #if HAS(MOTION_SENSOR) // CBA to inject SensorPool which would only be used to check for existence of the sensor
        add_view(new MenuBooleanSelectorView("Smart Alarm", alm->smart, [alm](bool newSmart) { alm->smart = newSmart; }));
        add_view(new MenuNumberSelectorView("Smart margin", 1, 30, 1, alm->smart_margin_minutes, activation, [alm](int newMargin) { alm->smart_margin_minutes = newMargin; }));
    #endif
        add_view(new MenuMelodySelectorView(b, "Melody", alm->melody_no, activation, [alm](int newMelodyNo) { alm->melody_no = newMelodyNo; }));
    }

    ~AlarmEditorView() {
        if(ts_view != nullptr) {
            delete ts_view;
        }
    }

    void step() {
        ListView::step();
        snprintf(time_str, 8, "%02d:%02d", setting->hour, setting->minute);
    }

private:
    char time_str[8] = { 0 };
    alarm_setting_t *setting;
    MenuTimeSettingView *ts_view;
};

AppShimAlarmEditor::AppShimAlarmEditor(Beeper *b): ProtoShimNavMenu::ProtoShimNavMenu() {
    beeper = b;
    current_editor = nullptr;
    current_editing_idx = 0;
    edit_flag = false;
    std::function<void(bool, Renderable*)> normalActivationFunction = [this](bool isActive, Renderable* instance) {
        if(isActive) push_renderable(instance, TRANSITION_NONE);
        else pop_renderable(TRANSITION_NONE);
    };
    std::function<void(bool, Renderable*)> pushPop = [this](bool isActive, Renderable* instance) {
        if(isActive) push_renderable(instance, TRANSITION_SLIDE_HORIZONTAL_LEFT);
        else pop_renderable(TRANSITION_SLIDE_HORIZONTAL_RIGHT);
    };


    std::function<void(int)> beginEditing = [this, normalActivationFunction, pushPop](int editIdx) {
        if(current_editor != nullptr) {
            delete current_editor;
        }
        edit_flag = true;
        current_editing_idx = editIdx;
        const alarm_setting_t * settings = get_alarm_list();
        current_editing_setting = settings[editIdx];
        current_editor = new AlarmEditorView(&current_editing_setting, beeper, normalActivationFunction, pushPop);
        push_renderable(current_editor, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    };

    for(int i = 0; i < ALARM_LIST_SIZE; i++) {
        main_menu->add_view(new AlarmItemView(i, beginEditing));
    }
    main_menu->add_view(new MenuNumberSelectorPreferenceView("Snooze time", PREFS_KEY_ALARM_SNOOZE_MINUTES, 0, 30, 1, normalActivationFunction));
    main_menu->add_view(new MenuNumberSelectorPreferenceView("Max beeping time, minutes", PREFS_KEY_ALARM_MAX_DURATION_MINUTES, 0, 120, 1, normalActivationFunction));
}

void AppShimAlarmEditor::pop_renderable(transition_type_t transition) {
    if(back_stack.size() == 0) {
        pop_state(STATE_ALARM_EDITOR, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        return;
    }

    if(back_stack.size() == 1) {
        // returning to alarm list, likely from editing screen
        if(current_editor != nullptr && edit_flag) {
            // cannot delete current_editor here because it's still used by the transition coordinator for animation
            edit_flag = false;
            set_alarm(current_editing_idx, current_editing_setting);
            main_menu->get_view(current_editing_idx)->prepare();
        }
    }
    ProtoShimNavMenu::pop_renderable(transition);
}
