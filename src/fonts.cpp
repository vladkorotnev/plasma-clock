#include <fonts.h>

// 1st-party fonts

// Utilitary font to draw the FVU progress bar
static const uint8_t one_pixel_bar_data[] = {
    // cursor, empty
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    // bar, filled
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
};

const font_definition_t one_pixel_bar_font = {
    .encoding = FONT_ENCODING_BESPOKE_ASCII,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = '|' - 1,
    .invalid_character = '|' - 1,
    .width = 1,
    .height = 8,
    .range_count = 1,
    .data = one_pixel_bar_data,
    .ranges = {
        font_range_t {
            .start = '|' - 1,
            .end = '|'
        }
    }
};


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

const font_definition_t fps_counter_font = {
    .encoding = FONT_ENCODING_BESPOKE_ASCII,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = 0x0,
    .invalid_character = '0',
    .width = 4,
    .height = 5,
    .range_count = 1,
    .data = fps_counter_font_data,
    .ranges = {
        font_range_t {
            .start = '0',
            .end = '9'
        }
    }
};

// 3rd-party fonts below
// SEE `font/FILE_ID.DIZ` FOR FONT AUTHORS' CREDITS

extern const uint8_t keyrus0808[] asm("_binary_src_font_keyrus08X08_FNT_start");
extern const uint8_t keyrus0816[] asm("_binary_src_font_keyrus08X16_FNT_start");
extern const uint8_t xnu_font_data[] asm("_binary_src_font_xnu_fnt_start");

const font_definition_t xnu_font = {
    .encoding = FONT_ENCODING_UTF16,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = 0x16,
    .invalid_character = '?',
    .width = 8,
    .height = 16,
    .range_count = 1,
    .data = xnu_font_data,
    .ranges = {
        { .start = 0, .end = 0xFF, .data_offset = 0 }, // dummy + ASCII Latin + Latin supplement
    }
};

const font_definition_t keyrus0808_font = {
    .encoding = FONT_ENCODING_UTF16,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = 0x16,
    .invalid_character = 0x0F,
    .width = 8,
    .height = 8,
    .range_count = 7,
    .data = keyrus0808,
    .ranges = {
        { .start = 0, .end = 0x1F, .data_offset = 0 }, // PIS-OS Legacy CP866 compat
        { .start = 0x20, .end = 0x7F, .data_offset = 0x20 * 8 }, // ASCII Latin
        { .start = 0x0410, .end = 0x043F, .data_offset = 0x80 * 8 }, // Cyrillic block 1 (А-п)
        { .start = 0x0440, .end = 0x044F, .data_offset = 0xE0 * 8 }, // Cyrillic block 2 (р-я)
        { .start = 0x0401, .end = 0x0401, .data_offset = 0x85 * 8 }, // Cyrillic Ё
        { .start = 0x0451, .end = 0x0451, .data_offset = 0xA5 * 8 }, // Cyrillic ё
        { .start = 0x00B0, .end = 0x00B0, .data_offset = 0xF8 * 8 }, // Degree sign
        { .start = 0x2190, .end = 0x2190, .data_offset = 0x1B * 8 }, // Left arrow
        { .start = 0x2191, .end = 0x2191, .data_offset = 0x18 * 8 }, // Up arrow
        { .start = 0x2192, .end = 0x2192, .data_offset = 0x1A * 8 }, // Right arrow
        { .start = 0x2193, .end = 0x2193, .data_offset = 0x19 * 8 }, // Down arrow
    }
};

const font_definition_t keyrus0816_font = {
    .encoding = FONT_ENCODING_UTF16,
    .glyph_format = SPRFMT_HORIZONTAL,
    .cursor_character = 0x16,
    .invalid_character = 0x0F,
    .width = 8,
    .height = 16,
    .range_count = 7,
    .data = keyrus0816,
    .ranges = {
        { .start = 0, .end = 0x1F, .data_offset = 0 }, // PIS-OS Legacy CP866 compat
        { .start = 0x20, .end = 0x7F, .data_offset = 0x20 * 16 }, // ASCII Latin
        { .start = 0x0410, .end = 0x043F, .data_offset = 0x80 * 16 }, // Cyrillic block 1 (А-п)
        { .start = 0x0440, .end = 0x044F, .data_offset = 0xE0 * 16 }, // Cyrillic block 2 (р-я)
        { .start = 0x0401, .end = 0x0401, .data_offset = 0x85 * 16 }, // Cyrillic Ё
        { .start = 0x0451, .end = 0x0451, .data_offset = 0xA5 * 16 }, // Cyrillic ё
        { .start = 0x00B0, .end = 0x00B0, .data_offset = 0xF8 * 16 }, // Degree sign
        { .start = 0x2190, .end = 0x2190, .data_offset = 0x1B * 16 }, // Left arrow
        { .start = 0x2191, .end = 0x2191, .data_offset = 0x18 * 16 }, // Up arrow
        { .start = 0x2192, .end = 0x2192, .data_offset = 0x1A * 16 }, // Right arrow
        { .start = 0x2193, .end = 0x2193, .data_offset = 0x19 * 16 }, // Down arrow
    }
};
