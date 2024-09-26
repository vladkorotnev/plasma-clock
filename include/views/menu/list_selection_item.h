#pragma once
#include <views/framework.h>
#include <service/prefs.h>
#include <functional>
#include <vector>

class MenuListSelectorView: public Composite {
public:
    bool isActive;
    MenuListSelectorView(
        const char * title, 
        std::vector<const char*> items, 
        int initialValueIndex, 
        std::function<void(bool, Renderable*)> onActivated,
        std::function<void(int)> onChange
    );
    ~MenuListSelectorView();

    void step() override;
    void render(FantaManipulator *fb) override;

protected:
    int currentValue;
    std::vector<const char*> items;
private:
    StringScroll * label;
    StringScroll * value;
    std::function<void(int)> onChange;
    std::function<void(bool, Renderable*)> onActivated;
};

class MenuListSelectorPreferenceView: public MenuListSelectorView {
public:
    MenuListSelectorPreferenceView(const char * title, std::vector<const char*> items, prefs_key_t prefs_key, std::function<void(bool, Renderable*)> onActivated, std::function<void(int)> onAfterChanged = [](int){}):
        key(prefs_key),
        MenuListSelectorView(
            title, 
            items, 
            prefs_get_int(prefs_key), 
            onActivated, 
            [this, onAfterChanged](int newVal) { prefs_set_int(key, newVal); onAfterChanged(newVal); }
        ) {}

    void step() {
        currentValue = prefs_get_int(key);
        MenuListSelectorView::step();
    }
private:
    prefs_key_t key;
};