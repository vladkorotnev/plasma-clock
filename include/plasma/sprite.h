#pragma once
#include <stdint.h>
#include <string.h>

/// @brief A piece of graphics, laid out horizontally. 
/// @details I.e. a 14x2 spritewill consist of 4 bytes. 0th one will contain the leftmost 6 pixels of the top row aligned towards LSB, 
///          1st one will contain the rightmost 8 pixels, 2nd one will contain the leftmost 6px of the bottom row, and so forth.
typedef struct sprite {
    uint8_t width, height;
    const uint8_t* data;
} sprite_t;

/// @brief An animated sprite. 
/// @see sprite_t
typedef struct ani_sprite {
    uint8_t width, height;
    /// @brief Number of frames in the animation
    uint8_t frames;
    /// @brief How many display frames should each animation frame be displayed for
    uint8_t screen_frames_per_frame;
    /// @brief How many display frames should the first frame be shown before the animation loops
    uint8_t holdoff_frames;
    const uint8_t* data;
} ani_sprite_t;

/// @brief A context of a playing animated sprite
typedef struct ani_sprite_state {
    /// @brief Currently playing sprite
    const ani_sprite_t* ani_sprite;
    /// @brief Display frames passed since the last animation frame was shown
    uint8_t framecount;
    /// @brief Index of the current animation frame
    uint8_t playhead;
    /// @brief Display frames remaining until the animation will restart
    int holdoff_counter;
} ani_sprite_state_t;

typedef uint8_t* fanta_buffer_t;

/// @brief Convert a horizontally laid out, LSB aligned sprite bitmap of an arbitrary size, to a vertically aligned 16-bit-per-column Fanta buffer. 
/// @note Unused pixels are filled with 0's. Transparency et al. should be handled by the drawing code.
extern fanta_buffer_t sprite_to_fanta(const sprite_t*);
/// @brief Offset a raw Fanta buffer vertically. Negative is towards the top.
extern void fanta_offset_y(fanta_buffer_t,int,size_t);

/// @brief Initialize a playback context for an animated sprite
extern ani_sprite_state_t ani_sprite_prepare(const ani_sprite*);
/// @brief Get the current animation frame to be drawn for an animated sprite
extern sprite_t ani_sprite_frame(ani_sprite_state_t*);