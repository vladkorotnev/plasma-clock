#pragma once
#include <functional>
#include <views/framework.h>
#include <input/keys.h>
#include <fonts.h>

class MenuActionItemView: public Renderable {
public:
    MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon = nullptr, key_id_t button = KEY_RIGHT);
    ~MenuActionItemView();

    void prepare();
    void step();
    void render(FantaManipulator *fb);
    void cleanup();

private:
    key_id_t _button;
    const sprite_t * _icon;
    StringScroll * label;
    std::function<void()> _action;
};
