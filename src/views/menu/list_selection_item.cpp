#include <views/menu/list_selection_item.h>
#include <input/keys.h>
#include <fonts.h>

MenuListSelectorView::MenuListSelectorView(const char * title, std::vector<const char*> items_, int initialValue, std::function<void(bool, Renderable*)> onActivated_, std::function<void(int)> onChange_):
    label(new StringScroll(&keyrus0808_font, title)),
    value(new StringScroll(&keyrus0808_font, items_[initialValue])),
    currentValue(initialValue),
    items(items_),
    onChange(onChange_),
    onActivated(onActivated_),
    isActive(false) {
        value->set_y_position(keyrus0808_font.height);
        value->align_to_right = true;
        add_subrenderable(label);
        add_subrenderable(value);
}

MenuListSelectorView::~MenuListSelectorView() {
    delete label;
}


void MenuListSelectorView::step() {
    Composite::step();

    if(isActive) {
        if(hid_test_key_state(KEY_DOWN) == KEYSTATE_HIT) {
            currentValue++;
            if(currentValue == items.size()) currentValue = 0;
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
        fb->put_glyph(&keyrus0808_font, 0x12 /* CP866: up-down arrows */, 0, 8);
    }
}
