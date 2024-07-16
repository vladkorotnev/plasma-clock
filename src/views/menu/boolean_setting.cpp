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
    fb->put_string(&keyrus0808_font, on_str, fb->get_width() - 48, 8, _currentValue);
    fb->put_string(&keyrus0808_font, off_str, fb->get_width() - 16, 8, !_currentValue);
}

void MenuBooleanSelectorView::cleanup() {
    label->cleanup();
}
