#pragma once
#include <views/framework.h>
#include <input/keys.h>
#include <service/prefs.h>
#include <fonts.h>
#include <service/localize.h>

class MenuBooleanSelectorView: public Renderable {
public:
    MenuBooleanSelectorView(const char * title, bool initialValue, std::function<void(bool)> onChange, key_id_t button = KEY_RIGHT);

    ~MenuBooleanSelectorView();

    void prepare() override;
    void step() override;
    void render(FantaManipulator *fb) override;
    void cleanup() override;

protected:
    bool _currentValue;
private:
    key_id_t _button;
    StringScroll * label;
    std::function<void(bool)> _onChange;
};

class MenuBooleanSettingView: public MenuBooleanSelectorView {
public:
    MenuBooleanSettingView(const char * title, prefs_key_t prefs_key, std::function<void(bool)> onAfterChange = [](bool){}, key_id_t button = KEY_RIGHT) :
        key(prefs_key),
        MenuBooleanSelectorView(
            title, 
            prefs_get_bool(prefs_key), 
            [this, onAfterChange](bool value) { 
                prefs_set_bool(key, value);
                onAfterChange(value);
            }, 
            button
        )
    {
    }

    void step() {
        _currentValue = prefs_get_bool(key);
        MenuBooleanSelectorView::step();
    }
private:
    prefs_key_t key;
};
