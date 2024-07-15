#include <app/alarm_editor.h>
#include <views/menu/menu.h>
#include <state.h>

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
        for(int x = 0; x <= 10; x++) {
            fb->plot_pixel(x, 2, true);
            fb->plot_pixel(x, 12, true);
            if(ALARM_IS_ENABLED(setting)) fb->plot_pixel(x, 3, true);
            if(x == 0 || x == 10 || (ALARM_IS_ENABLED(setting) && x == 1)) {
                for(int y = 2; y < 12; y++) fb->plot_pixel(x, y, true);
            }
        }
        fb->put_string(&keyrus0808_font, index_str, 2, 4, ALARM_IS_ENABLED(setting));

        fb->put_string(&keyrus0808_font, time_buf, 14, 0);

        if(setting.days == ALARM_DAY_GLOBAL_ENABLE || setting.days == 0) {
            fb->put_string(&keyrus0808_font, "One Time", 14, 8);
        } else {
            for(int d = 0; d < 7; d++) {
                fb->put_string(&keyrus0808_font, &days[d * 2], 14 + d * (keyrus0808_font.width + 1), 8, ALARM_ON_DAY(setting, d));
            }
        }
    }

    void step() {
        snprintf(time_buf, 6, "%02d:%02d", setting.hour, setting.minute);

        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            _action(index);
        }
    }

private:
    std::function<void(int)> _action;
    alarm_setting_t setting;
    int index;
    char index_str[4];
    char time_buf[6];
    char days[14] = {
        'S', 0,
        'M', 0,
        'T', 0,
        'W', 0,
        'T', 0,
        'F', 0,
        'S', 0,
    };
};

class AppShimAlarmEditor::AlarmEditorView: public ListView {
public:
    AlarmEditorView(alarm_setting_t *alm, Beeper *b, std::function<void(bool, Renderable*)> activation): ListView::ListView() {
        add_view(new MenuBooleanSelectorView("Enabled", ALARM_IS_ENABLED(*alm), [alm](bool newEnabled) {
            if(newEnabled) {
                alm->days |= ALARM_DAY_GLOBAL_ENABLE;
            } else {
                alm->days &= ~ALARM_DAY_GLOBAL_ENABLE;
            }
        }));

        add_view(new MenuInfoItemView("Days", "TODO"));
        add_view(new MenuInfoItemView("Time", "TODO"));
        add_view(new MenuMelodySelectorView(b, "Melody", alm->melody_no, activation, [alm](int newMelodyNo) { alm->melody_no = newMelodyNo; }));
    }
};

AppShimAlarmEditor::AppShimAlarmEditor(Beeper *b): ProtoShimNavMenu::ProtoShimNavMenu() {
    beeper = b;
    current_editor = nullptr;
    current_editing_idx = 0;
    std::function<void(bool, Renderable*)> normalActivationFunction = [this](bool isActive, Renderable* instance) {
        if(isActive) push_renderable(instance, TRANSITION_NONE);
        else pop_renderable(TRANSITION_NONE);
    };

    std::function<void(int)> beginEditing = [this, normalActivationFunction](int editIdx) {
        if(current_editor != nullptr) {
            delete current_editor;
        }
        current_editing_idx = editIdx;
        const alarm_setting_t * settings = get_alarm_list();
        current_editing_setting = settings[editIdx];
        current_editor = new AlarmEditorView(&current_editing_setting, beeper, normalActivationFunction);
        push_renderable(current_editor, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    };

    for(int i = 0; i < ALARM_LIST_SIZE; i++) {
        main_menu->add_view(new AlarmItemView(i, beginEditing));
    }
    main_menu->add_view(new MenuNumberSelectorPreferenceView("Snooze time", PREFS_KEY_ALARM_SNOOZE_MINUTES, 1, 30, 1, normalActivationFunction));
}

void AppShimAlarmEditor::pop_renderable(transition_type_t transition) {
    if(back_stack.size() == 0) {
        pop_state(STATE_ALARM_EDITOR, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        return;
    }

    if(back_stack.size() == 1) {
        // returning to alarm list, likely from editing screen
        if(current_editor != nullptr) {
            set_alarm(current_editing_idx, current_editing_setting);
            main_menu->get_view(current_editing_idx)->prepare();
        }
    }
    ProtoShimNavMenu::pop_renderable(transition);
}
