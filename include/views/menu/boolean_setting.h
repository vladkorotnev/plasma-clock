#pragma once
#include <views/framework.h>
#include <input/keys.h>
#include <service/prefs.h>
#include <fonts.h>

class MenuBooleanSelectorView: public Renderable {
public:
    MenuBooleanSelectorView(const char * title, bool initialValue, std::function<void(bool)> onChange, key_id_t button = KEY_RIGHT);

    ~MenuBooleanSelectorView();

    void prepare() override;
    void step() override;
    void render(FantaManipulator *fb) override;
    void cleanup() override;

private:
    const char * on_str = "Yes";
    const char * off_str = "No";
    key_id_t _button;
    StringScroll * label;
    bool _currentValue;
    std::function<void(bool)> _onChange;
};

class MenuBooleanSettingView: public MenuBooleanSelectorView {
public:
    MenuBooleanSettingView(const char * title, prefs_key_t key, key_id_t button = KEY_RIGHT) :
        MenuBooleanSelectorView(title, prefs_get_bool(key), [key](bool value) { prefs_set_bool(key, value); }, button)
    {
    }
};
