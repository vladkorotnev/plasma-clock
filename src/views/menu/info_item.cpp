#include <views/menu/info_item.h>

MenuInfoItemView::MenuInfoItemView(const char * title, const char * subtitle) {
    top_label = new StringScroll(&keyrus0808_font, title);
    top_label->set_y_position(0);

    bottom_label = new StringScroll(&keyrus0808_font, subtitle);
    bottom_label->set_y_position(keyrus0808_font.height);

    top_label->start_at_visible = true;
    top_label->holdoff = 100;
    bottom_label->start_at_visible = true;
    bottom_label->holdoff = 100;
    bottom_label->align_to_right = true;

    add_composable(top_label);
    add_composable(bottom_label);
}

MenuInfoItemView::~MenuInfoItemView() {
    delete top_label;
    delete bottom_label;
}
