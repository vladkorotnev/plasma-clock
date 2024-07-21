#pragma once
#include <functional>
#include <views/framework.h>
#include <input/keys.h>
#include <fonts.h>

class MenuActionItemView: public Composite {
public:
    MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon = nullptr, const char * subtitle = nullptr);
    ~MenuActionItemView();

    void step();
    void render(FantaManipulator *fb);

private:
    const sprite_t * _icon;
    StringScroll * label;
    StringScroll * sublabel;
    const char * subtitle;
    std::function<void()> _action;
};
