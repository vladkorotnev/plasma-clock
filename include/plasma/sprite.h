#pragma once
#include <stdint.h>
#include <string.h>

typedef struct sprite {
    uint8_t width, height;
    const uint8_t* data;
} sprite_t;

typedef uint8_t* fanta_buffer_t;

extern fanta_buffer_t sprite_to_fanta(const sprite_t*);
extern void fanta_offset_y(fanta_buffer_t,int,size_t);