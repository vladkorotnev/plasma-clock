#include <views/idle_screens/fb2k.h>
#include <service/foo_client.h>
#include <graphics/font.h>
#include <rsrc/common_icons.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "FOO";


Fb2kView::Fb2kView() {
    font = find_font(FONT_STYLE_UI_TEXT);

    last_update = 0;

    bottom_line = new StringScroll(font);
    bottom_line->x_offset = 17;
    bottom_line->set_y_position(8);
    add_composable(bottom_line);

    top_line = new StringScroll(font);
    top_line->x_offset = 17;
    top_line->set_y_position(0);
    add_composable(top_line);

    icon = new SpriteView(&music_icns);
    icon->width = 16;
    add_composable(icon);

    track_nr = new StringScroll(find_font(FONT_STYLE_HUD_DIGITS));
    track_nr->render_mode = TEXT_OUTLINED | TEXT_NO_BACKGROUND;
    track_nr->stopped = true;
    track_nr->set_y_position(10);
    track_nr->hidden = true;
    add_composable(track_nr);

    top_line->holdoff = 50;
    bottom_line->holdoff = 50;

    ESP_LOGV(LOG_TAG, "Init");
}

void Fb2kView::update_if_needed() {
    if(!foo_is_playing()) return;

    TickType_t ts = foo_last_recv();
    if(ts != last_update) {
        last_update = ts;
        
        char artist_buffer[128] = { 0 };
        char title_buffer[128] = { 0 };
        foo_get_artist(artist_buffer, 128);
        foo_get_title(title_buffer, 128);
        int trk_no = foo_get_track_number();

        ESP_LOGI(LOG_TAG, "New now playing info: %i: %s - %s", trk_no, artist_buffer, title_buffer);
        top_line->set_string(artist_buffer);
        bottom_line->set_string(title_buffer);
        if(trk_no <= 0) {
            track_nr->hidden = true;
        } else {
            track_nr->hidden = false;
            char tnbuf[4] = { 0 };
            snprintf(tnbuf, 4, "%02d", trk_no);
            tnbuf[3] = 0;
            track_nr->set_string(tnbuf);
            track_nr->x_offset = icon->x_offset + icon->width - track_nr->string_width + 1;
        }

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
        if(foo_last_recv() != last_update) {
            ESP_LOGV(LOG_TAG, "Requesting attention");
            return DISP_TIME_ATTENTION;
        } else {
            int estimated_ms = (bottom_line->estimated_frame_count() * 1000 / 58);
            return estimated_ms > 5000 ? estimated_ms : DISP_TIME_NO_OVERRIDE;
        }
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}