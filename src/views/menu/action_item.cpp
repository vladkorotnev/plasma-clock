#include <views/menu/action_item.h>

MenuActionItemView::MenuActionItemView(const char * title, std::function<void()> action, const sprite_t * icon, const char * subtitle_) {
    subtitle = subtitle_;
    label = new StringScroll((subtitle == nullptr) ? find_font(FONT_STYLE_TALL_TEXT) : find_font(FONT_STYLE_UI_TEXT), title);
    label->start_at_visible = true;
    label->holdoff = 240;
    add_composable(label);
    if(subtitle != nullptr) {
        sublabel = new StringScroll(find_font(FONT_STYLE_UI_TEXT), subtitle);
        sublabel->set_y_position(label->font->height);
        sublabel->align_to_right = true;
        sublabel->start_at_visible = true;
        sublabel->holdoff = 240;
        add_composable(sublabel);
    } else {
        sublabel = nullptr;
    }
    if(icon != nullptr) {
        iconView = new SpriteView(icon);
        label->x_offset = 17;
        if(sublabel != nullptr) sublabel->x_offset = 17;
        add_composable(iconView);
    } else {
        iconView = nullptr;
    }
    _action = action;
}

MenuActionItemView::~MenuActionItemView() {
    delete label;
    if(sublabel != nullptr) {
        delete sublabel;
    }
    if(iconView != nullptr) {
        delete iconView;
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
