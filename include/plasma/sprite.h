#pragma once
#include <stdint.h>
#include <string.h>

typedef struct sprite {
    uint8_t width, height;
    const uint8_t* data;
} sprite_t;

typedef struct ani_sprite {
    uint8_t width, height;
    uint8_t frames;
    uint8_t screen_frames_per_frame;
    const uint8_t* data;
} ani_sprite_t;

typedef struct ani_sprite_state {
    const ani_sprite_t* ani_sprite;
    uint8_t framecount;
    uint8_t playhead;
} ani_sprite_state_t;

typedef uint8_t* fanta_buffer_t;

extern fanta_buffer_t sprite_to_fanta(const sprite_t*);
extern void fanta_offset_y(fanta_buffer_t,int,size_t);

extern ani_sprite_state_t ani_sprite_prepare(const ani_sprite*);
extern sprite_t ani_sprite_frame(ani_sprite_state_t*);