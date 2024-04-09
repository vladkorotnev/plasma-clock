#include "plasma/sprite.h"
#include <string.h>
#include <algorithm>
#include <Arduino.h>

static char LOG_TAG[] = "FB_SPR";

fanta_buffer_t sprite_to_fanta(const sprite_t* sprite) {
    size_t required_size = sprite->width;

    uint16_t * columns = (uint16_t*) malloc(required_size * sizeof(uint16_t));
    if(columns == nullptr) {
        ESP_LOGE(LOG_TAG, "Allocation failed");
        return nullptr;
    }

    memset(columns, 0, required_size * sizeof(uint16_t));

    for(int i = 0; i < sprite->width; i++) {
        for(int j = std::min(sprite->height, (uint8_t) 16) - 1; j >= 0; j--) {
            columns[i] <<= 1;
            uint8_t cur_byte = sprite->data[j*(std::max((sprite->width/8), 1)) + i / 8];
            if(sprite->width < 8) {
                // move the bits towards MSB
                cur_byte <<= (8 - sprite->width);
            }
            columns[i] |= (cur_byte >> (7 - (i % 8)) & 1);
        }
    }

    return (uint8_t*) columns;
}

void fanta_offset_y(fanta_buffer_t fanta, int vert_offset, size_t width) {
    uint16_t * columns = (uint16_t*) fanta;
    for(int i = 0; i < width; i++) {
        if(vert_offset > 0) {
            columns[i] <<= vert_offset;
        } else if(vert_offset < 0) {
            columns[i] >>= abs(vert_offset);
        }
    }
}

ani_sprite_state_t ani_sprite_prepare(const ani_sprite* as) {
    return ani_sprite_state_t {
        .ani_sprite = as,
        .framecount = 0,
        .playhead = 0
    };
}

sprite_t ani_sprite_frame(ani_sprite_state_t* as) {
    size_t frame_data_idx = as->playhead * std::max(1, as->ani_sprite->width/8) * as->ani_sprite->height;
    const uint8_t* cur_frame = &as->ani_sprite->data[frame_data_idx];

    as->framecount++;
    if(as->framecount == as->ani_sprite->screen_frames_per_frame) {
        as->framecount = 0;
        as->playhead++;
        if(as->playhead == as->ani_sprite->frames) {
            as->playhead = 0;
        }
    }

    return sprite_t {
        .width = as->ani_sprite->width,
        .height = as->ani_sprite->height,
        .data = cur_frame
    };
}