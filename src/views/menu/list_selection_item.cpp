#include <views/menu/list_selection_item.h>
#include <input/keys.h>
#include <graphics/font.h>

MenuListSelectorView::MenuListSelectorView(const char * title, std::vector<const char*> items_, int initialValue, std::function<void(bool, Renderable*)> onActivated_, std::function<void(int)> onChange_):
    label(new StringScroll(find_font(FONT_STYLE_UI_TEXT), title)),
    value(new StringScroll(find_font(FONT_STYLE_UI_TEXT))),
    currentValue(initialValue),
    items(items_),
    onChange(onChange_),
    onActivated(onActivated_),
    isActive(false) {
        if(initialValue < items_.size()) {
            value->set_string(items_[initialValue]);
        } else if(items_.size() >= 1) {
            value->set_string(items_[0]);
        }
        value->set_y_position(label->font->height);
        value->align_to_right = true;
        value->start_at_visible = true;
        value->holdoff = 100;
        value->left_margin = 10;
        label->start_at_visible = true;
        label->holdoff = 100;
        add_composable(label);
        add_composable(value);
}

MenuListSelectorView::~MenuListSelectorView() {
    delete label;
    delete value;
}


void MenuListSelectorView::step() {
    Composite::step();

    if(isActive) {
        if(hid_test_key_state_repetition(KEY_DOWN) == KEYSTATE_HIT) {
            currentValue++;
            if(currentValue >= items.size()) currentValue = 0;
            onChange(currentValue);
            value->set_string(items[currentValue]);
        } else if(hid_test_key_state_repetition(KEY_UP) == KEYSTATE_HIT) {
            currentValue--;
            if(currentValue < 0) currentValue = items.size() - 1;
            onChange(currentValue);
            value->set_string(items[currentValue]);
        } else if(hid_test_key_state(KEY_LEFT)) {
            onActivated(false, this);
            isActive = false;
        }
    } else {
        if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
            onActivated(true, this);
            isActive = true;
        }
    }
}

void MenuListSelectorView::render(FantaManipulator *fb) {
    Composite::render(fb);
    if(isActive) {
        fb->put_glyph(value->font, 0x2195 /* '↕' UTF16 */, 0, 8);
    }
}
