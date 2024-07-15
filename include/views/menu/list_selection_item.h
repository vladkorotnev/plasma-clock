#pragma once
#include <views/framework.h>
#include <service/prefs.h>
#include <functional>
#include <vector>

class MenuListSelectorView: public Composite {
public:
    MenuListSelectorView(const char * title, std::vector<const char*> items, int initialValueIndex, std::function<void(bool, Renderable*)> onActivated, std::function<void(int)> onChange);
    ~MenuListSelectorView();

    void step() override;
    void render(FantaManipulator *fb) override;

private:
    bool isActive;
    StringScroll * label;
    StringScroll * value;
    int currentValue;
    std::function<void(int)> onChange;
    std::function<void(bool, Renderable*)> onActivated;
    std::vector<const char*> items;
};

class MenuListSelectorPreferenceView: public MenuListSelectorView {
public:
    MenuListSelectorPreferenceView(const char * title, std::vector<const char*> items, prefs_key_t prefs_key, std::function<void(bool, Renderable*)> onActivated):
        MenuListSelectorView(title, items, prefs_get_int(prefs_key), onActivated, [prefs_key](int newVal) { prefs_set_int(prefs_key, newVal); }) {}
};