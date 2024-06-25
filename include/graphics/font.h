#pragma once
#include <stdint.h>
#include "sprite.h"

/// @brief A monospace bitmap font in a contiguous character space
typedef struct font_definition {
    /// @brief The first character included in the font
    char16_t start_character;
    /// @brief The last character included in the font
    char16_t end_character;
    /// @brief The character that best suits the cursor role in the font
    char16_t cursor_character;
    /// @brief Width of the character in pixels
    uint8_t width;
    /// @brief Height of the character in pixels
    uint8_t height;
    /// @brief Bitmap data of the fonts, laid out horizontally, aligned towards LSB.
    /// I.e. a 4x5 font will have 5 bytes per character, with each byte representing a single horizontal row of pixels, with bits counted M-to-L and aligned towards the LSB.
    const uint8_t* data;
} font_definition_t;

/// @brief Fetch a sprite from a font based on it's character code
extern sprite_t sprite_from_glyph(const font_definition_t*, char16_t glyph);
/// @brief Measure a string's width when drawn with a specified font 
extern unsigned int measure_string_width(const font_definition_t*, const char*);