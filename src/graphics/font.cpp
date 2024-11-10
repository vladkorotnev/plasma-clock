#include <graphics/font.h>
#include <string.h>
#include <Arduino.h>

static char LOG_TAG[] = "FONT";

void sprite_from_glyph(const font_definition_t* font, char16_t glyph, bool masked, sprite_t* output) {
    bool found = false;
    const uint8_t * glyph_ptr = nullptr;
    for(int i = 0; i < font->range_count; i++) {
        const font_range_t *r = &font->ranges[i];
        if(glyph >= r->start && glyph <= r->end) {
            found = true;
            glyph_ptr = &font->data[r->data_offset + (glyph - r->start) * font->height * std::max(font->width/8, 1)];
            break;
        }
    }

    if(!found) {
        if(glyph != font->invalid_character) {
            ESP_LOGI(LOG_TAG, "Not known character %x in font", glyph);
            sprite_from_glyph(font, font->invalid_character, masked, output);
        } else {
            ESP_LOGW(LOG_TAG, "Not known character %x in font, and swap failed!", glyph);
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
    unsigned int rslt = 0;
    const char * tmp = s;
    while(iterate_utf8(&tmp)) {
        rslt += f->width;
    }
    return rslt + ((attributes & TEXT_OUTLINED) != 0 ? 2 : 0);
}

bool _utf8_continuation_get(const char ** ptr, char32_t * dst) {
    if((**ptr & 0b11000000) != 0b10000000) {
        ESP_LOGE(LOG_TAG, "Malformed UTF8 sequence at 0x%x: expected continuation, got %x", *ptr, **ptr);
        return false;
    }
    *dst <<= 6;
    *dst |= (**ptr & 0b00111111);
    (*ptr)++;
    return true;
}

char16_t iterate_utf8(const char ** ptr) {
    if(*ptr == 0) return 0;

    char32_t val = **ptr;

    if(val <= 0x7F) {
        // 1 byte case
        (*ptr)++;
    } else if((val & 0b11100000) == 0b11000000) {
        // 2 byte case
        val &= 0b00011111;
        (*ptr)++;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
    } else if((val & 0b11110000) == 0b11100000) {
        // 3 byte case
        val &= 0b00001111;
        (*ptr)++;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
    } else if((val & 0b11111000) == 0b11110000) {
        // 4 byte case
        val &= 0b00000111;
        (*ptr)++;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
        if(!_utf8_continuation_get(ptr, &val)) return 0;
    } else {
        ESP_LOGE(LOG_TAG, "Unsupported UTF8 sequence at 0x%x (value = %x)", *ptr, **ptr);
        return 0;
    }

    return (val & 0xFFFF);
}