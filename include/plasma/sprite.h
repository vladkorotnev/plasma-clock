#pragma once
#include <stdint.h>
#include <string.h>

typedef struct sprite {
    uint8_t width, height;
    const uint8_t* data;
} sprite_t;

extern uint8_t* sprite_to_fanta(const sprite_t*);
extern void fanta_offset_y(uint8_t*,int,size_t);