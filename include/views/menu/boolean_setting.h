#pragma once
#include <views/framework.h>
#include <input/keys.h>
#include <service/prefs.h>
#include <fonts.h>

class MenuBooleanSettingView: public Renderable {
public:
    MenuBooleanSettingView(const char * title, prefs_key_t key, key_id_t button = KEY_RIGHT) {
        label = new StringScroll(&keyrus0808_font, title);
        _key = key;
        _button = button;
    }

    ~MenuBooleanSettingView() {
        delete label;
    }

    void prepare() {
        label->prepare();
        current_value = prefs_get_bool(_key);
    }

    void step() {
        label->step();

        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            current_value = !current_value;
            prefs_set_bool(_key, current_value);
        }
    }

    void render(FantaManipulator *fb) {
        label->render(fb);
        fb->put_string(&keyrus0808_font, current_value ? on_str : off_str, fb->get_width() - measure_string_width(&keyrus0808_font, current_value ? on_str : off_str), 8);
    }

    void cleanup() {
        label->cleanup();
    }

private:
    const char * on_str = "Yes";
    const char * off_str = "No";
    prefs_key_t _key;
    key_id_t _button;
    StringScroll * label;
    bool current_value;
};