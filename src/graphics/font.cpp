#include <graphics/font.h>
#include <string.h>
#include <Arduino.h>
#include <utils.h>
#include <service/disk.h>
#include <service/localize.h>

static char LOG_TAG[] = "FONT";

#define MONOFONT_CUR_VERSION 0x0001
#define MONOFONT_MAGIC_HDR 0x6F466F4D // 'MoFo', MonoFont little endian
#define MONOFONT_MAGIC_RANGES 0x73676E52 // 'Rngs' Range table marker
#define MONOFONT_MAGIC_RANGES_DEFL 0x5A676E52 // 'RngZ' Range table marker
#define MONOFONT_MAGIC_BITMAP 0x73504D42 // 'BMPs' Bitmap section marker
#define MONOFONT_MAGIC_BITMAP_DEFL 0x5A504D42 // 'BMPZ' Deflated Bitmap section marker

typedef struct __attribute__((packed)) mofo_header {
    const uint32_t magic;
    const uint16_t version;
    const font_encoding_t encoding;
    const sprite_fmt_t glyph_format;
    const char16_t cursor_character;
    const char16_t invalid_character;
    const uint8_t width;
    const uint8_t height;
} mofo_header_t;

typedef struct __attribute__((packed)) mofo_section {
    const uint32_t magic;
    const uint32_t size;
    union {
        const uint32_t real_size;
        const uint32_t item_count;
    }; // uncompressed size for BMPZ or RngZ, count of items for Rngs
} mofo_section_t;

bool load_font_from_file_handle(FILE * f, font_definition_t * dest) {
    dest->valid = false;

    size_t r = 0;
    mofo_header_t head = { 0 };
    mofo_section_t sect = { 0 };
    long pos = ftell(f);
    r = fread(&head, 1, sizeof(mofo_header_t), f);
    if(r != sizeof(mofo_header_t)) {
        ESP_LOGE(LOG_TAG, "Header read error, wanted %i bytes, got %i", sizeof(mofo_header_t), r);
        return false;
    }

    if(head.magic != MONOFONT_MAGIC_HDR) {
        ESP_LOGE(LOG_TAG, "Expected header magic to be 0x%x, got 0x%x", MONOFONT_MAGIC_HDR, head.magic);
        return false;
    }

    if(head.version != MONOFONT_CUR_VERSION) {
        ESP_LOGE(LOG_TAG, "Expected header version to be 0x%x, got 0x%x", MONOFONT_CUR_VERSION, head.version);
        return false;
    }

    dest->encoding = head.encoding;
    dest->glyph_format = head.glyph_format;
    dest->cursor_character = head.cursor_character;
    dest->invalid_character = head.invalid_character;
    dest->width = head.width;
    dest->height = head.height;

    pos = ftell(f);
    r = fread(&sect, 1, sizeof(mofo_section_t), f);
    if(r != sizeof(mofo_section_t)) {
        ESP_LOGE(LOG_TAG, "Section read error, wanted %i bytes, got %i", sizeof(mofo_section_t), r);
        return false;
    }

    if(sect.magic != MONOFONT_MAGIC_RANGES && sect.magic != MONOFONT_MAGIC_RANGES_DEFL) {
        ESP_LOGE(LOG_TAG, "Expected range table section at 0x%x (magic to be 0x%x or 0x%x, got 0x%x)", pos, MONOFONT_MAGIC_RANGES, MONOFONT_MAGIC_RANGES_DEFL, sect.magic);
        return false;
    }

    font_range_t * ranges = (font_range_t*) ps_malloc(sect.size);
    if(ranges == nullptr) {
        ESP_LOGE(LOG_TAG, "OOM allocating range table");
        return false;
    }

    size_t total = 0;
    while(total < sect.size && r > 0) {
        r = fread(&((uint8_t*) ranges)[total], 1, sect.size - total, f);
        total += r;
    }

    if(total != sect.size) {
        ESP_LOGE(LOG_TAG, "Underrun reading range table: expected %u bytes but got only %u", sect.size, total);
        free(ranges);
        return false;
    }

    if(sect.magic == MONOFONT_MAGIC_RANGES_DEFL) {
        ESP_LOGI(LOG_TAG, "Decompressing RngZ");
        ranges = (font_range_t *) decompress_emplace(ranges, sect.size, sect.real_size);
        if(ranges == nullptr) return false;
        dest->range_count =  (sect.real_size / sizeof(font_range_t));
    } else {
        dest->range_count = sect.item_count;
    }

    dest->ranges = ranges;
    for(int i = 0; i < dest->range_count; i++) {
        const font_range_t * r = &dest->ranges[i];
        ESP_LOGV(LOG_TAG, "  - Glyph range: start = 0x%x, end = 0x%x, bitmap offset = 0x%x", r->start, r->end, r->data_offset);
    }

    pos = ftell(f);
    r = fread(&sect, 1, sizeof(mofo_section_t), f);
    if(r != sizeof(mofo_section_t)) {
        ESP_LOGE(LOG_TAG, "Section read error, wanted %i bytes, got %i", sizeof(mofo_section_t), r);
        return false;
    }

    if(sect.magic != MONOFONT_MAGIC_BITMAP && sect.magic != MONOFONT_MAGIC_BITMAP_DEFL) {
        ESP_LOGE(LOG_TAG, "Expected bitmap table section at 0x%x (magic to be 0x%x or 0x%x, got 0x%x)", pos, MONOFONT_MAGIC_BITMAP, MONOFONT_MAGIC_BITMAP_DEFL, sect.magic);
        return false;
    }

    uint8_t * bitmap = (uint8_t*) ps_malloc(sect.size);
    if(bitmap == nullptr) {
        ESP_LOGE(LOG_TAG, "OOM allocating bitmap table");
        free(ranges);
        return false;
    }

    total = 0;
    while(total < sect.size && r > 0) {
        r = fread(&bitmap[total], 1, sect.size - total, f);
        total += r;
    }
    
    if(total != sect.size) {
        ESP_LOGE(LOG_TAG, "Underrun reading bitmap table: expected %u bytes but got only %u", sect.size, total);
        free(bitmap);
        free(ranges);
        return false;
    }

    if(sect.magic == MONOFONT_MAGIC_BITMAP_DEFL) {
        ESP_LOGI(LOG_TAG, "Decompressing BMPZ");
        bitmap = (uint8_t*) decompress_emplace(bitmap, sect.size, sect.real_size);
        if(bitmap == nullptr) {
            free(ranges);
            return false;
        }
    }

    dest->data = bitmap;
    dest->valid = true;

    ESP_LOGI(LOG_TAG, "Got font: encoding=%x, glyphfmt=%x, cursor=%x, invalid=%x, w=%u, h=%u, range_cnt=%u, data=0x%x, ranges=0x%x", dest->encoding, dest->glyph_format, dest->cursor_character, dest->invalid_character, dest->width, dest->height, dest->range_count, dest->data, dest->ranges);
    return true;
}

bool load_font_from_file(const char * path, font_definition_t * dest) {
    ESP_LOGI(LOG_TAG, "Load %s", path);
    FILE * f = nullptr;
    f = fopen(path, "rb");
    if(!f) {
        ESP_LOGE(LOG_TAG, "File open error: %s (%i, %s)", path, errno, strerror(errno));
        return false;
    }

    fseek(f, 0, SEEK_SET);
    bool rslt = load_font_from_file_handle(f, dest);
    fclose(f);
    return rslt;
}

static bool fonts_loaded = false;
static font_definition_t keyrus0808_font = { .valid = false };
static font_definition_t keyrus0816_font = { .valid = false };
static font_definition_t xnu_font = { .valid = false };
static font_definition_t misaki_font = { .valid = false };
static font_definition_t jiskan_font = { .valid = false };

static const std::vector<const font_definition_t*> half_height_fonts = { &keyrus0808_font, &misaki_font };
static const std::vector<const font_definition_t*> full_height_fonts = { &keyrus0816_font, &xnu_font, &jiskan_font };

// FPS counter font (inspired by FRAPS)
static const uint8_t fps_counter_font_data[] = {
    0b01110,
    0b01010,
    0b01010,
    0b01010,
    0b01110,

    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,

    0b01110,
    0b00010,
    0b01110,
    0b01000,
    0b01110,

    0b01110,
    0b00010,
    0b00110,
    0b00010,
    0b01110,

    0b01010,
    0b01010,
    0b01110,
    0b00010,
    0b00010,

    0b01110,
    0b01000,
    0b01110,
    0b00010,
    0b01110,

    0b01110,
    0b01000,
    0b01110,
    0b01010,
    0b01110,

    0b01110,
    0b00010,
    0b00010,
    0b00010,
    0b00010,

    0b01110,
    0b01010,
    0b01110,
    0b01010,
    0b01110,

    0b01110,
    0b01010,
    0b01110,
    0b00010,
    0b01110,
};

const font_range_t fps_ctr_font_ranges[] = {
    font_range_t {
        .start = '0',
        .end = '9',
        .data_offset = 0
    }
};
const font_definition_t fps_counter_font = {
    .valid = true,
    .encoding = FONT_ENCODING_BESPOKE_ASCII,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = 0x0,
    .invalid_character = '0',
    .width = 4,
    .height = 5,
    .range_count = 1,
    .data = fps_counter_font_data,
    .ranges = fps_ctr_font_ranges
};

bool load_fonts() {
    if(fonts_loaded) return true;
    bool rslt = true;
    // To save FS blocks for smaller fonts for now, we have a `default.mofo` which is just keyrus0808 + keyrus0816 + xnu concatenated
    FILE * f = nullptr;
    f = fopen(FS_MOUNTPOINT "/font/default.mofo", "rb");
    if(!f) {
        ESP_LOGE(LOG_TAG, "Default font collection open error (%i, %s)", errno, strerror(errno));
        return false;
    }

    fseek(f, 0, SEEK_SET);
    if(!load_font_from_file_handle(f, &keyrus0808_font)) rslt = false;
    if(!load_font_from_file_handle(f, &keyrus0816_font)) rslt = false;
    if(!load_font_from_file_handle(f, &xnu_font)) rslt = false;
    fclose(f);
    if(!rslt) return false;

    // and now the humongous CJK fonts which rightfully take their own FS blocks
    if(!load_font_from_file(FS_MOUNTPOINT "/font/misaki_mincho.mofo", &misaki_font)) return false;
    if(!load_font_from_file(FS_MOUNTPOINT "/font/jiskan16.mofo", &jiskan_font)) return false;

    fonts_loaded = true;

    return true;
}

static bool did_try_loading_font = false;
const font_definition_t * find_font(font_style_t style, font_fallback_behavior_t fallbacks) {
    // Stub, todo fallback and all
    if(!fonts_loaded && !did_try_loading_font) load_fonts();
    did_try_loading_font = true;

    if(style == FONT_STYLE_UI_TEXT || style == FONT_STYLE_CLOCK_FACE_SMALL || style == FONT_STYLE_CONSOLE) {
        return &keyrus0808_font;
    }
    else if(style == FONT_STYLE_TALL_TEXT) {
        if(active_display_language() == DSPL_LANG_JA) {
            return &jiskan_font;
        } else {
            return &keyrus0816_font;
        }
    }
    else if(style == FONT_STYLE_CLOCK_FACE) {
        return &xnu_font;
    }
    else if(style == FONT_STYLE_HUD_DIGITS) {
        return &fps_counter_font;
    }

    return nullptr;
}


const uint8_t* char_data_ptr_for_font(const font_definition_t* font, char16_t glyph) {
    const uint8_t * glyph_ptr = nullptr;

    int low = 0;
    int high = font->range_count - 1;
    while(low <= high) {
        int mid = low + (high - low) / 2;
        const font_range_t * range = &font->ranges[mid];
        if(glyph >= range->start && glyph <= range->end) {
            int idx = 0;
            switch(font->glyph_format) {
                case SPRFMT_HORIZONTAL:
                    idx = range->data_offset + (glyph - range->start) * std::max((font->width / 8), 1) * font->height;
                    break;
                case SPRFMT_NATIVE:
                default:
                    idx = range->data_offset + (glyph - range->start) * std::max((font->height / 8), 1) * font->width;
                    break;
            }
            glyph_ptr = &font->data[idx];
            break;
        } else if (glyph < range->start) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return glyph_ptr;
}

/// @brief Dummy glyph of 8x8
const uint16_t dummy_char_data[] = {
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

void sprite_from_glyph(const font_definition_t* font, char16_t glyph, bool masked, sprite_t* output, font_fallback_behavior_t fallbacks) {
    if(font == nullptr || !font->valid) {
        output->width = 8;
        output->height = 8;
        output->data = (const uint8_t*) dummy_char_data;
        output->mask = masked ? (const uint8_t*) dummy_char_data : nullptr;
        output->format = SPRFMT_NATIVE;
        return;
    }

    const uint8_t * glyph_ptr = char_data_ptr_for_font(font, glyph);

    if(glyph_ptr == nullptr) {
        bool substituted = false;
        if(glyph != font->invalid_character) {
            if((fallbacks & FONT_FALLBACK_SAME_HEIGHT) != 0) {
                // Someday this will be a proper font DB
                // For now with this qty of fonts we can just iterate lol
                const std::vector<const font_definition_t *>* alt_fonts = nullptr;
                switch(font->height) {
                    case 8:
                        alt_fonts = &half_height_fonts;
                        break;
                    case 16:
                        alt_fonts = &full_height_fonts;
                        break;
                    default:
                        break;
                }

                if(alt_fonts != nullptr) {
                    for(auto alt_font: *alt_fonts) {
                        ESP_LOGV(LOG_TAG, "Look for substitute of char %x in font at 0x%x", glyph, alt_font->ranges);
                        const uint8_t * alt_glyph = char_data_ptr_for_font(alt_font, glyph);
                        if(alt_glyph != nullptr) {
                            substituted = true;
                            glyph_ptr = alt_glyph;
                            font = alt_font;
                            break;
                        }
                    }
                }
            }
            
            if(!substituted) {
                ESP_LOGV(LOG_TAG, "Not known character %x in font", glyph);
                sprite_from_glyph(font, font->invalid_character, masked, output, fallbacks);
            }
        } else {
            ESP_LOGW(LOG_TAG, "Not known character %x in font, and swap failed!", glyph);
            output->width = 8;
            output->height = 8;
            output->data = (const uint8_t*) dummy_char_data;
            output->mask = masked ? (const uint8_t*) dummy_char_data : nullptr;
            output->format = SPRFMT_NATIVE;
        }
        if(!substituted)
            return;
    }
    output->width = font->width;
    output->height = font->height;
    output->data = glyph_ptr;
    output->mask = masked ? glyph_ptr : nullptr;
    output->format = font->glyph_format;
}

unsigned int measure_string_width(const font_definition_t* f, const char* s, text_attributes_t attributes) {
    if(f == nullptr || !f->valid) return 0;
    unsigned int rslt = 0;
    const char * tmp = s;
    sprite_t spr;
    while(char16_t uchar = iterate_utf8(&tmp)) {
        sprite_from_glyph(f, uchar, false, &spr, FONT_FALLBACK_SAME_HEIGHT);
        rslt += spr.width;
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