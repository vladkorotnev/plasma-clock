#include <device_config.h>

#if HAS(WORDNIK_API)

#include <views/word_of_the_day.h>
#include <service/wordnik.h>
#include <fonts.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "WOTD";

static const uint8_t spr_book_data[] = {
    // By PiiXL
    // https://piiixl.itch.io/mega-1-bit-icons-bundle
    0x00, 0x00, 0x00, 0x00, 0x78, 0x3c, 0xfc, 0x7e, 0x86, 0xc2, 0xfa, 0xbe, 0x86, 0xc2, 0xfa, 0xbe, 
    0x86, 0xc2, 0xfa, 0xbe, 0xfe, 0xfe, 0x8e, 0xe2, 0x02, 0x80, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x04, 0x78, 0x3c, 0xfc, 0x76, 0x86, 0xce, 0xfa, 0xb6, 0x86, 0xce, 0xfa, 0xb6, 
    0x86, 0xce, 0xfa, 0xbe, 0xfe, 0xfa, 0x8e, 0xe2, 0x02, 0x80, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x00, 0x18, 0x00, 0x38, 0x78, 0x68, 0xfc, 0xda, 0x86, 0xaa, 0xfa, 0xda, 0x86, 0xaa, 0xfa, 0xda, 
    0x86, 0xba, 0xfa, 0xf6, 0xfe, 0xee, 0x8e, 0xc2, 0x02, 0x80, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x00, 0xc0, 0x00, 0xe0, 0x78, 0xac, 0xfc, 0xc6, 0x86, 0xa2, 0xfa, 0xc6, 0x86, 0xa2, 0xfa, 0xc6, 
    0x86, 0x62, 0xfa, 0x26, 0xfe, 0x2e, 0x8e, 0x42, 0x02, 0x00, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x03, 0x00, 0x03, 0x80, 0x72, 0x9c, 0xf3, 0x1e, 0x82, 0x82, 0xf3, 0x1e, 0x82, 0x82, 0xf3, 0x1e, 
    0x83, 0x82, 0xf1, 0x9e, 0xf0, 0x9e, 0x80, 0x82, 0x01, 0x00, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x0c, 0x00, 0x0e, 0x00, 0x4a, 0x3c, 0xcc, 0x7e, 0x8a, 0x42, 0xcc, 0x3e, 0x8a, 0x42, 0xcc, 0x3e, 
    0x8e, 0x42, 0xce, 0x3e, 0xc2, 0x7e, 0x82, 0x62, 0x02, 0x00, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x00, 0x00, 0x1c, 0x00, 0x5e, 0x3c, 0xd2, 0x7e, 0x9e, 0xc2, 0xd2, 0xbe, 0x9e, 0xc2, 0xd2, 0xbe, 
    0x9e, 0xc2, 0xde, 0xbe, 0xc6, 0xfe, 0x82, 0xe2, 0x02, 0x80, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x78, 0x3c, 0xfc, 0x7e, 0x86, 0xc2, 0xfa, 0xbe, 0x86, 0xc2, 0xfa, 0xbe, 
    0x86, 0xc2, 0xfa, 0xbe, 0xfe, 0xfe, 0x8e, 0xe2, 0x02, 0x80, 0xfc, 0x7e, 0x03, 0x80, 0x00, 0x00
};

static const ani_sprite_t book_icon = {
    .width = 16, .height = 16,
    .frames = 8,
    .screen_frames_per_frame = 4,
    .holdoff_frames = 8,
    .data = spr_book_data
};

WordOfTheDayView::WordOfTheDayView() {
    font = &keyrus0808_font;
    memset(word_buffer, 0, 32);
    memset(definition_buffer, 0, 256);

    strncpy(word_buffer, "Loading...", 31);
    strncpy(definition_buffer, "Transfer (a program or data) into memory, or into the central processor from storage.", 255);

    last_update = 0;
    icon_state = ani_sprite_prepare(&book_icon);

    bottom_line = new StringScroll(font);
    bottom_line->set_y_position(8);
    bottom_line->set_string(definition_buffer);

    current_icon_frame = { 0 };

    ESP_LOGV(LOG_TAG, "Init");
}

WordOfTheDayView::~WordOfTheDayView() {
    delete bottom_line;
}

void WordOfTheDayView::prepare() {
    TickType_t ts = wotd_get_last_update();
    if(ts != last_update) {
        last_update = ts;
        wotd_get_current(word_buffer, 32, definition_buffer, 256);
        ESP_LOGI(LOG_TAG, "New word of the day");
    }

    icon_state = ani_sprite_prepare(&book_icon);
    bottom_line->set_string(definition_buffer);
}

void WordOfTheDayView::step() {
    current_icon_frame = ani_sprite_frame(&icon_state);
}

void WordOfTheDayView::render(FantaManipulator *fb) {
    if(current_icon_frame.data != nullptr)
        fb->put_sprite(&current_icon_frame, 0, 0);
    
    FantaManipulator * text_window = fb->slice(17, fb->get_width() - 17);
    text_window->put_string(font, word_buffer, 0, 0);
    bottom_line->render(text_window);
    delete text_window;
}

int WordOfTheDayView::desired_display_time() {
    return (bottom_line->estimated_frame_count() * 1000 / 58) + 2000; 
}

#endif