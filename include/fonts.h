#pragma once
#include <plasma/font.h>

extern const uint8_t keyrus0808[] asm("_binary_src_font_keyrus08X08_FNT_start");
extern const uint8_t keyrus0816[] asm("_binary_src_font_keyrus08X16_FNT_start");
extern const uint8_t sg8bit[] asm("_binary_src_font_sg8x8_fnt_start");
extern const uint8_t xnu_font_data[] asm("_binary_src_font_xnu_fnt_start");

extern const font_definition_t xnu_font;
extern const font_definition_t keyrus0808_font;
extern const font_definition_t keyrus0816_font;
extern const font_definition_t sg8bit_font;