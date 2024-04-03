#pragma once
#include <stdint.h>
#include "sprite.h"

typedef struct font_definition {
    char16_t start_character;
    char16_t end_character;
    char16_t cursor_character;
    uint8_t width, height;
    const uint8_t* data;
} font_definition_t;

extern sprite_t sprite_from_glyph(const font_definition_t*, char16_t glyph);