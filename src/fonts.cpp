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
    .start_character = '|' - 1,
    .end_character = '|',
    .cursor_character = '|' - 1,
    .width = 1,
    .height = 8,
    .data = one_pixel_bar_data
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
    .start_character = '0',
    .end_character = '9',
    .cursor_character = 0x0,
    .width = 4,
    .height = 5,
    .data = fps_counter_font_data
};

// 3rd-party fonts below
// SEE `font/FILE_ID.DIZ` FOR FONT AUTHORS' CREDITS

extern const uint8_t keyrus0808[] asm("_binary_src_font_keyrus08X08_FNT_start");
extern const uint8_t keyrus0816[] asm("_binary_src_font_keyrus08X16_FNT_start");
extern const uint8_t xnu_font_data[] asm("_binary_src_font_xnu_fnt_start");

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
