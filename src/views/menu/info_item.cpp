#include <views/menu/info_item.h>

MenuInfoItemView::MenuInfoItemView(const char * title, const char * subtitle) {
    top_label = new StringScroll(&keyrus0808_font, title);
    top_label->set_y_position(0);

    bottom_label = new StringScroll(&keyrus0808_font, subtitle);
    bottom_label->set_y_position(keyrus0808_font.height);

    add_subrenderable(top_label);
    add_subrenderable(bottom_label);
}

MenuInfoItemView::~MenuInfoItemView() {
    delete top_label;
    delete bottom_label;
}
