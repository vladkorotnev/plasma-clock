#pragma once
#include "wipe.h"
#include "slide.h"

typedef enum transition_type: uint8_t {
    TRANSITION_NONE = 0,
    TRANSITION_WIPE,
    TRANSITION_SLIDE_HORIZONTAL_LEFT,
    TRANSITION_SLIDE_HORIZONTAL_RIGHT,
    TRANSITION_SLIDE_VERTICAL_UP,
    TRANSITION_SLIDE_VERTICAL_DOWN,

    TRANSITION_RANDOM,
} transition_type_t;


static Transition* transition_type_to_transition(transition_type_t type) {  
    switch(type) {
        case TRANSITION_WIPE: return new WipeTransition();
        case TRANSITION_SLIDE_HORIZONTAL_LEFT:  return new SlideTransition(false);
        case TRANSITION_SLIDE_HORIZONTAL_RIGHT:  return new SlideTransition(true);
        case TRANSITION_SLIDE_VERTICAL_UP: return new VerticalSlideTransition(false);
        case TRANSITION_SLIDE_VERTICAL_DOWN: return new VerticalSlideTransition(true);
        case TRANSITION_RANDOM: return transition_type_to_transition( (transition_type_t) ((esp_random() % (TRANSITION_RANDOM - 1)) + 1));

        case TRANSITION_NONE:
        default:
            return nullptr;
    }
}