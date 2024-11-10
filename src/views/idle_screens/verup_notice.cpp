#include "views/idle_screens/verup_notice.h"
#include <network/netmgr.h>
#include <fonts.h>
#include <service/localize.h>
#include <service/httpfvu.h>
#include <service/prefs.h>
#include <state.h>

static const uint8_t icon_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8, 0x7a, 0xbc, 0x75, 0x5c, 0x77, 0xdc, 0x7b, 0xbc, 0x3b, 0xb8, 
	0x1d, 0x70, 0x0d, 0x60, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x03, 0x80
};

static const sprite_t icon_spr = {
    .width = 16, .height = 16,
    .data = icon_data, .mask = nullptr,
    .format = SPRFMT_HORIZONTAL
};

int VerUpNoticeView::desired_display_time() {
    httpfvu_new_version_info_t ver = get_current_version_info();
    if(!prefs_get_bool(PREFS_KEY_FVU_AUTO_CHECK) || ver.fs_current == nullptr || ver.fs_new == nullptr || strcmp(ver.fs_current, ver.fs_new) == 0) 
        return DISP_TIME_DONT_SHOW;
    return DISP_TIME_NO_OVERRIDE;
}

void VerUpNoticeView::create_components_if_needed() {
    if(title == nullptr) {
        title = new StringScroll(&keyrus0808_font, nullptr);
        title->x_offset = 17;
        title->holdoff = 60;
        title->start_at_visible = true;
        add_composable(title);
    }

    if(message == nullptr) {
        message = new StringScroll(&keyrus0808_font, nullptr);
        message->set_y_position(8);
        message->x_offset = 17;
        message->holdoff = 60;
        message->start_at_visible = true;
        add_composable(message);
    }

    if(icon == nullptr) {
        icon = new SpriteView(&icon_spr);
        add_composable(icon);
    }
}

void VerUpNoticeView::prepare() {
    create_components_if_needed();
    Screen::prepare();

    title->set_string(localized_string("New Firmware Available!"));
    message->set_string(localized_string("Press → to install"));

    int width = std::max(title->string_width, message->string_width);
    title->string_width = width;
    message->string_width = width;
}

void VerUpNoticeView::step() {
    Screen::step();
#if HAS(HTTPFVU)
    if(hid_test_key_state(KEY_RIGHT)) {
        push_state(STATE_HTTPFVU, TRANSITION_SLIDE_HORIZONTAL_LEFT);
    }
#endif
}
