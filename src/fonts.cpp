#include <fonts.h>

const font_definition_t xnu_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .width = 8,
    .height = 16,
    .data = xnu_font_data
};

const font_definition_t keyrus0808_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .width = 8,
    .height = 8,
    .data = keyrus0808
};

const font_definition_t keyrus0814_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .width = 8,
    .height = 14,
    .data = keyrus0814
};

const font_definition_t keyrus0816_font = {
    .start_character = 0x0,
    .end_character = 0xFF,
    .width = 8,
    .height = 16,
    .data = keyrus0816
};