#include <views/menu/boolean_setting.h>

MenuBooleanSelectorView::MenuBooleanSelectorView(const char * title, bool initialValue, std::function<void(bool)> onChange, key_id_t button) :
    _button(button),
    label(new StringScroll(&keyrus0808_font, title)),
    _currentValue(initialValue),
    _onChange(onChange)
{
    label->start_at_visible = true;
    label->holdoff = 100;
}

MenuBooleanSelectorView::~MenuBooleanSelectorView() {
    delete label;
}

void MenuBooleanSelectorView::prepare() {
    label->prepare();
}

void MenuBooleanSelectorView::step() {
    label->step();

    if(hid_test_key_state(_button) == KEYSTATE_HIT) {
        _currentValue = !_currentValue;
        _onChange(_currentValue);
    }
}

void MenuBooleanSelectorView::render(FantaManipulator *fb) {
    label->render(fb);
    fb->put_string(&keyrus0808_font, on_str, fb->get_width() - measure_string_width(&keyrus0808_font, off_str) - measure_string_width(&keyrus0808_font, on_str) - 16, 8, _currentValue ? TEXT_INVERTED : TEXT_NORMAL);
    fb->put_string(&keyrus0808_font, off_str, fb->get_width() - measure_string_width(&keyrus0808_font, off_str), 8, !_currentValue ? TEXT_INVERTED : TEXT_NORMAL);
}

void MenuBooleanSelectorView::cleanup() {
    label->cleanup();
}
