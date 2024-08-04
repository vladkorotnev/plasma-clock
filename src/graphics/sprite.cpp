#include "graphics/sprite.h"
#include <string.h>
#include <algorithm>
#include <Arduino.h>
#include <map>

static char LOG_TAG[] = "FB_SPR";

void* gralloc(const size_t size) {
#ifndef BOARD_HAS_PSRAM
    return malloc(size);
#else
    return heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
#endif
}

ani_sprite_state_t ani_sprite_prepare(const ani_sprite* as) {
    return ani_sprite_state_t {
        .ani_sprite = as,
        .framecount = 0,
        .playhead = 0,
        .holdoff_counter = as->holdoff_frames * as->screen_frames_per_frame
    };
}

sprite_t ani_sprite_frame(ani_sprite_state_t* as) {
    size_t frame_data_idx;
    
    if(as->holdoff_counter == 0) {
        frame_data_idx = as->playhead * std::max(1, as->ani_sprite->width/8) * as->ani_sprite->height;

        as->framecount++;
        if(as->framecount == as->ani_sprite->screen_frames_per_frame) {
            as->framecount = 0;
            as->playhead++;
            if(as->playhead == as->ani_sprite->frames) {
                as->playhead = 0;
                as->holdoff_counter = as->ani_sprite->holdoff_frames * as->ani_sprite->screen_frames_per_frame;
            }
        }
    } else {
        frame_data_idx = 0;
        as->holdoff_counter--;
    }

    const uint8_t* cur_frame = &as->ani_sprite->data[frame_data_idx];
    return sprite_t {
        .width = as->ani_sprite->width,
        .height = as->ani_sprite->height,
        .data = cur_frame,
        .mask = nullptr,
        .format = as->ani_sprite->format
    };
}