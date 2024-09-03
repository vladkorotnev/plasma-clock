#include <graphics/font.h>
#include <string.h>
#include <Arduino.h>

static char LOG_TAG[] = "FONT";

sprite_t sprite_from_glyph(const font_definition_t* font, char16_t glyph, bool masked) {
    if(glyph > font->end_character || glyph < font->start_character) {
        ESP_LOGI(LOG_TAG, "Not known character %u in font", glyph);
        glyph = 0;
    }
    size_t start_idx = (glyph - font->start_character) * font->height * std::max(font->width/8, 1);

    sprite_t rslt = {
        .width = font->width,
        .height = font->height,
        .data = &font->data[start_idx],
        .mask = masked ? &font->data[start_idx] : nullptr,
        .format = SPRFMT_HORIZONTAL
    };

    return rslt;
}

unsigned int measure_string_width(const font_definition_t* f, const char* s, text_attributes_t attributes) {
    size_t i = 0;
    unsigned int rslt = 0;
    while(char ch = s[i]) {
        // hacky way of mapping cyrillic from utf8 to cp866 since most of our fonts are from keyrus
        if((ch & 0b11100000) == 0b11000000 && (s[i + 1] & 0b11000000) == 0b10000000) {
            if(((ch >> 2) & 7) == 0x04) {
                // U+04xx
                ch = ((ch & 0b00000011) << 6) | (s[i + 1] & 0b00111111);
                i++;
                if(ch >= 0x10 && ch <= 0x3F) {
                    ch += (0x80 - 0x10);
                }
                else if(ch >= 0x40 && ch <= 0x4F) {
                    ch += (0xE0 - 0x40);
                }
                else if(ch == 0x01) {
                    ch = 0xF0;
                }
                else if(ch == 0x51) {
                    ch = 0xF1;
                }
                else {
                    // wasn't cyrillic, so draw as is...
                    i--;
                    ch = s[i];
                }
            }
        }
        rslt += f->width;
        i++;
    }
    return rslt + ((attributes & TEXT_OUTLINED) != 0 ? 2 : 0);
}