#pragma once
#include <functional>
#include <views/framework.h>
#include <input/keys.h>
#include <graphics/font.h>

class MenuActionItemView: public Composite {
public:
    MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon = nullptr, const char * subtitle = nullptr);
    ~MenuActionItemView();

    void step();

private:
    StringScroll * label;
    StringScroll * sublabel;
    SpriteView * iconView;
    const char * subtitle;
    std::function<void()> _action;
};
