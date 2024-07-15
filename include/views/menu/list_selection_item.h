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

protected:
    int currentValue;
private:
    bool isActive;
    StringScroll * label;
    StringScroll * value;
    std::function<void(int)> onChange;
    std::function<void(bool, Renderable*)> onActivated;
    std::vector<const char*> items;
};

class MenuListSelectorPreferenceView: public MenuListSelectorView {
public:
    MenuListSelectorPreferenceView(const char * title, std::vector<const char*> items, prefs_key_t prefs_key, std::function<void(bool, Renderable*)> onActivated):
        key(prefs_key),
        MenuListSelectorView(title, items, prefs_get_int(prefs_key), onActivated, [this](int newVal) { prefs_set_int(key, newVal); }) {}

    void step() {
        currentValue = prefs_get_bool(key);
        MenuListSelectorView::step();
    }
private:
    prefs_key_t key;
};