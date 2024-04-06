#include <fonts.h>

extern const uint8_t keyrus0808[] asm("_binary_src_font_keyrus08X08_FNT_start");
extern const uint8_t keyrus0816[] asm("_binary_src_font_keyrus08X16_FNT_start");
extern const uint8_t sg8bit[] asm("_binary_src_font_sg8x8_fnt_start");
extern const uint8_t xnu_font_data[] asm("_binary_src_font_xnu_fnt_start");

const font_definition_t sg8bit_font = {
    .start_character = 0x20,
    .end_character = 0xFF,
    .cursor_character = 0x80,
    .width = 8,
    .height = 8,
    .data = sg8bit
};

const font_definition_t xnu_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .cursor_character = 0x16,
    .width = 8,
    .height = 16,
    .data = xnu_font_data
};

const font_definition_t keyrus0808_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .cursor_character = 0x16,
    .width = 8,
    .height = 8,
    .data = keyrus0808
};

const font_definition_t keyrus0816_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .cursor_character = 0x16,
    .width = 8,
    .height = 16,
    .data = keyrus0816
};

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
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
    0b10000000,
};

const font_definition_t one_pixel_bar_font = {
    .start_character = '|' - 1,
    .end_character = '|',
    .cursor_character = '|' - 1,
    .width = 1,
    .height = 8,
    .data = one_pixel_bar_data
};