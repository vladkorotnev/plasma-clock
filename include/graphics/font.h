#pragma once
#include <stdint.h>
#include "sprite.h"

typedef enum font_encoding {
    /// @brief ASCII or some bespoke encoding (e.g. graphical fonts)
    FONT_ENCODING_BESPOKE_ASCII,
    /// @brief UTF-16 codepoints
    FONT_ENCODING_UTF16,
} font_encoding_t;

typedef struct font_range {
    /// @brief The first character included in the range
    char16_t start;
    /// @brief The last character included in the range
    char16_t end;
    /// @brief Offset in the data block where the characters are uniformly, contiguously located
    size_t data_offset;
} font_range_t;

/// @brief A monospace bitmap font in an uncontiguous character space
typedef struct font_definition {
    /// @brief In which encoding are the character ranges specified?
    font_encoding_t encoding;
    /// @brief In which format are the glyphs stored?
    sprite_fmt_t glyph_format;
    /// @brief The character that best suits the cursor role in the font
    char16_t cursor_character;
    /// @brief The character to draw when an unknown character is requested
    char16_t invalid_character;
    /// @brief Width of the character in pixels
    uint8_t width;
    /// @brief Height of the character in pixels
    uint8_t height;
    /// @brief Count of character ranges in the font
    uint8_t range_count;
    /// @brief Bitmap data of the fonts, laid out horizontally, aligned towards LSB.
    /// I.e. a 4x5 font will have 5 bytes per character, with each byte representing a single horizontal row of pixels, with bits counted M-to-L and aligned towards the LSB.
    const uint8_t* data;
    /// @brief Character ranges, ordered low to high
    const font_range_t ranges[];
} font_definition_t;

typedef uint8_t text_attributes_t;

typedef enum text_style : text_attributes_t {
    /// @brief Normal text (black background, white text)
    TEXT_NORMAL = 0,
    /// @brief Render only text without background
    TEXT_NO_BACKGROUND = 1 << 0,
    /// @brief Render white text with a black outline.
    TEXT_OUTLINED = 1 << 1,
    /// @brief Invert the color of the text (white background, black text)
    TEXT_INVERTED = 1 << 2,
    /// @brief Invert the color of the outline (white instead of black)
    OUTLINE_INVERTED = 1 << 3,
} text_style_t;

/// @brief Fetch a sprite from a font based on it's character code
/// @param masked Whether the character should have no background (masked by itself)
void sprite_from_glyph(const font_definition_t*, char16_t glyph, bool masked, sprite_t* output);
/// @brief Measure a string's width when drawn with a specified font 
unsigned int measure_string_width(const font_definition_t*, const char*, text_attributes_t attributes = TEXT_NORMAL);
/// @brief Iterate over a UTF8 string's codepoints. Returns char16_t's and increments ptr until EOS or an error, otherwise returns 0.
char16_t iterate_utf8(const char ** ptr);