#include <views/menu/action_item.h>

MenuActionItemView::MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon, key_id_t button) {
    label = new StringScroll(&keyrus0816_font, title);
    _button = button;
    _action = action;
    _icon = icon;
}

MenuActionItemView::~MenuActionItemView() {
    delete label;
}

void MenuActionItemView::prepare() {
    label->prepare();
}

void MenuActionItemView::step() {
    label->step();

    if(hid_test_key_state(_button) == KEYSTATE_HIT) {
        _action();
    }
}

void MenuActionItemView::render(FantaManipulator *fb) {
    if(_icon == nullptr) {
        label->render(fb);
    } else {
        fb->put_sprite(_icon, 0, 0);
        FantaManipulator *offset = fb->slice(_icon->width + 1, fb->get_width() - _icon->width - 1);
        label->render(offset);
        delete offset;
    }
}

void MenuActionItemView::cleanup() {
    label->cleanup();
}
