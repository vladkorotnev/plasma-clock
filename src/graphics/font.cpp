#include <graphics/font.h>
#include <string.h>
#include <Arduino.h>

static char LOG_TAG[] = "FONT";

sprite_t sprite_from_glyph(const font_definition_t* font, char16_t glyph) {
    if(glyph > font->end_character || glyph < font->start_character) {
        ESP_LOGI(LOG_TAG, "Not known character %u in font", glyph);
        glyph = 0;
    }
    size_t start_idx = (glyph - font->start_character) * font->height * std::max(font->width/8, 1);

    sprite_t rslt = {
        .width = font->width,
        .height = font->height,
        .data = &font->data[start_idx]
    };

    return rslt;
}

extern unsigned int measure_string_width(const font_definition_t* f, const char* s) {
    return strlen(s) * f->width;
}