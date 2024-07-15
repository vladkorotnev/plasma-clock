#pragma once
#include <views/framework.h>
#include <service/prefs.h>
#include <functional>
#include <vector>

class MenuNumberSelectorView: public Composite {
public:
    MenuNumberSelectorView(const char * title, int min, int max, int step, int initialValue, std::function<void(bool, Renderable*)> onActivated, std::function<void(int)> onChange);
    ~MenuNumberSelectorView();

    void step() override;
    void render(FantaManipulator *fb) override;

private:
    bool isActive;
    StringScroll * label;
    StringScroll * value;
    char currentValueStr[9];
    int currentValue;
    int min;
    int max;
    int increment;
    std::function<void(int)> onChange;
    std::function<void(bool, Renderable*)> onActivated;
};

class MenuNumberSelectorPreferenceView: public MenuNumberSelectorView {
public:
    MenuNumberSelectorPreferenceView(const char * title, prefs_key_t prefs_key, int min, int max, int step, std::function<void(bool, Renderable*)> onActivated):
        MenuNumberSelectorView(title, min, max, step, prefs_get_int(prefs_key), onActivated, [prefs_key](int newVal) { prefs_set_int(prefs_key, newVal); }) {}
};
