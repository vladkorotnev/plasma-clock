#include <views/idle_screens/fb2k.h>
#include <service/foo_client.h>
#include <fonts.h>
#include <rsrc/common_icons.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "FOO";


Fb2kView::Fb2kView() {
    font = &keyrus0808_font;
    memset(artist_buffer, 0, 128);
    memset(title_buffer, 0, 128);

    last_update = 0;

    bottom_line = new StringScroll(font);
    bottom_line->x_offset = 17;
    bottom_line->set_y_position(8);
    bottom_line->set_string(title_buffer);
    add_composable(bottom_line);

    top_line = new StringScroll(font);
    top_line->x_offset = 17;
    top_line->set_y_position(0);
    top_line->set_string(artist_buffer);
    add_composable(top_line);

    icon = new SpriteView(&music_icns);
    icon->width = 16;
    add_composable(icon);

    top_line->holdoff = 50;
    bottom_line->holdoff = 50;

    ESP_LOGV(LOG_TAG, "Init");
}

void Fb2kView::update_if_needed() {
    if(!foo_is_playing()) return;

    TickType_t ts = foo_last_recv();
    if(ts != last_update) {
        last_update = ts;
        
        foo_get_artist(artist_buffer, 128);
        foo_get_title(title_buffer, 128);

        ESP_LOGI(LOG_TAG, "New now playing info: %s - %s", artist_buffer, title_buffer);
        top_line->set_string(artist_buffer);
        bottom_line->set_string(title_buffer);

        int width = std::max(top_line->string_width, bottom_line->string_width);
        top_line->string_width = width;
        bottom_line->string_width = width;
    }
}

void Fb2kView::prepare() {
    update_if_needed();
    Screen::prepare();
}

void Fb2kView::step() {
    update_if_needed();
    Screen::step();
}

int Fb2kView::desired_display_time() {
    if(foo_is_playing()) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}