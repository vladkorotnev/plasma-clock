#include "views/idle_screens/softap.h"
#include <network/netmgr.h>
#include <fonts.h>

static const uint8_t icon_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0xfe, 0x01, 0x01, 0x01, 0x55, 0x01, 0x01, 0x01, 0x55, 0x11, 0x01, 0x19, 0x51, 
    0xfd, 0x01, 0x19, 0x01, 0x11, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7d, 0x01, 0x01, 0x00, 0xfe
};

static const sprite_t icon_spr = {
    .width = 16, .height = 16,
    .data = icon_data, .mask = nullptr,
    .format = SPRFMT_HORIZONTAL
};

int SoftApInfoView::desired_display_time() {
    if(!NetworkManager::is_softAP()) return DISP_TIME_DONT_SHOW;
    return DISP_TIME_NO_OVERRIDE;
}

void SoftApInfoView::create_components_if_needed() {
    if(network_name == nullptr) {
        network_name = new StringScroll(&keyrus0808_font, nullptr);
        network_name->x_offset = 17;
        network_name->holdoff = 60;
        network_name->start_at_visible = true;
        add_composable(network_name);
    }

    if(ip_address == nullptr) {
        ip_address = new StringScroll(&keyrus0808_font, nullptr);
        ip_address->set_y_position(8);
        ip_address->x_offset = 17;
        ip_address->holdoff = 60;
        ip_address->start_at_visible = true;
        add_composable(ip_address);
    }

    if(icon == nullptr) {
        icon = new SpriteView(&icon_spr);
        add_composable(icon);
    }
}

void SoftApInfoView::prepare() {
    create_components_if_needed();
    Screen::prepare();

    network_name->set_string(NetworkManager::network_name());
    ip_address->set_string(NetworkManager::current_ip().c_str());

    int width = std::max(network_name->string_width, ip_address->string_width);
    network_name->string_width = width;
    ip_address->string_width = width;
}