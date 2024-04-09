#pragma once
#include "transitions/wipe.h"
#include "transitions/slide.h"

typedef enum transition_type: uint8_t {
    TRANSITION_NONE = 0,
    TRANSITION_WIPE,
    TRANSITION_SLIDE_HORIZONTAL,
    TRANSITION_SLIDE_VERTICAL,

    TRANSITION_RANDOM,
} transition_type_t;


static Transition* transition_type_to_transition(transition_type_t type) {  
    static WipeTransition wipe_transition = WipeTransition();
    static SlideTransition h_slide_transition = SlideTransition();
    static VerticalSlideTransition v_slide_transition = VerticalSlideTransition();

    switch(type) {
        case TRANSITION_WIPE: return &wipe_transition;
        case TRANSITION_SLIDE_HORIZONTAL:  return &h_slide_transition;
        case TRANSITION_SLIDE_VERTICAL: return &v_slide_transition;
        case TRANSITION_RANDOM: return transition_type_to_transition( (transition_type_t) ((esp_random() % (TRANSITION_RANDOM - 1)) + 1));

        case TRANSITION_NONE:
        default:
            return nullptr;
    }
}