#include <views/idle_screens/fb2k.h>
#include <service/foo_client.h>
#include <fonts.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "FOO";

static const uint8_t icn_music_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x07, 0xfc, 0x07, 0xfc, 0x07, 0xc4, 0x04, 0x04, 0x04, 0x04, 
    0x04, 0x04, 0x04, 0x3c, 0x3c, 0x74, 0x74, 0x64, 0x64, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const sprite_t icn_music = {
    .width = 16, .height = 16, .data = icn_music_data, .mask = nullptr
};

Fb2kView::Fb2kView() {
    font = &keyrus0808_font;
    memset(artist_buffer, 0, 128);
    memset(title_buffer, 0, 128);

    last_update = 0;

    bottom_line = new StringScroll(font);
    bottom_line->set_y_position(8);
    bottom_line->set_string(title_buffer);

    top_line = new StringScroll(font);
    top_line->set_y_position(0);
    top_line->set_string(artist_buffer);

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
}

void Fb2kView::step() {
    update_if_needed();
}

void Fb2kView::render(FantaManipulator *fb) {
    fb->put_sprite(&icn_music, 0, 0);
    
    FantaManipulator * text_window = fb->slice(15, fb->get_width() - 15);
    top_line->render(text_window);
    bottom_line->render(text_window);
    delete text_window;
}

int Fb2kView::desired_display_time() {
    if(foo_is_playing()) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}