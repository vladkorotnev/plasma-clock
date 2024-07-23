#include <views/menu/action_item.h>

MenuActionItemView::MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon, const char * subtitle_) {
    subtitle = subtitle_;
    label = new StringScroll((subtitle == nullptr) ? &keyrus0816_font : &keyrus0808_font, title);
    label->start_at_visible = true;
    label->holdoff = 240;
    add_subrenderable(label);
    if(subtitle != nullptr) {
        sublabel = new StringScroll(&keyrus0808_font, subtitle);
        sublabel->set_y_position(keyrus0808_font.height);
        sublabel->align_to_right = true;
        sublabel->start_at_visible = true;
        sublabel->holdoff = 240;
        add_subrenderable(sublabel);
    } else {
        sublabel = nullptr;
    }
    _action = action;
    _icon = icon;
}

MenuActionItemView::~MenuActionItemView() {
    delete label;
    if(sublabel != nullptr) {
        delete sublabel;
    }
}

void MenuActionItemView::step() {
    if(sublabel != nullptr && subtitle != nullptr) {
        // dynamically update subtitle because the underlying value might have changed
        sublabel->set_string(subtitle);
    }

    Composite::step();

    if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
        _action();
    }
}

void MenuActionItemView::render(FantaManipulator *fb) {
    if(_icon == nullptr) {
        Composite::render(fb);
    } else {
        fb->put_sprite(_icon, 0, 0);
        FantaManipulator *offset = fb->slice(_icon->width + 1, fb->get_width() - _icon->width - 1);
        Composite::render(offset);
        delete offset;
    }
}
