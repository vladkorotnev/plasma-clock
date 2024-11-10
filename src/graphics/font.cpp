#include <graphics/font.h>
#include <string.h>
#include <Arduino.h>

static char LOG_TAG[] = "FONT";

void sprite_from_glyph(const font_definition_t* font, char16_t glyph, bool masked, sprite_t* output) {
    bool found = false;
    const uint8_t * glyph_ptr = nullptr;
    for(int i = 0; i < font->range_count; i++) {
        const font_range_t *r = &font->ranges[i];
        if(glyph >= r->start || glyph <= r->end) {
            found = true;
            glyph_ptr = &font->data[r->data_offset + (glyph - r->start) * font->height * std::max(font->width/8, 1)];
            break;
        }
    }

    if(!found) {
        if(glyph != font->invalid_character) {
            ESP_LOGI(LOG_TAG, "Not known character %u in font", glyph);
            sprite_from_glyph(font, font->invalid_character, masked, output);
        } else {
            ESP_LOGW(LOG_TAG, "Not known character %u in font, and swap failed!", glyph);
            sprite_from_glyph(font, font->ranges[0].start, masked, output);
        }
        return;
    } else {
        output->width = font->width;
        output->height = font->height;
        output->data = glyph_ptr;
        output->mask = masked ? glyph_ptr : nullptr;
        output->format = font->glyph_format;
    }
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