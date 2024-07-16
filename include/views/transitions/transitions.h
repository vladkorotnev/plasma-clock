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
    static WipeTransition wipe_transition = WipeTransition();
    static SlideTransition h_slide_transition = SlideTransition(false);
    static SlideTransition rev_h_slide_transition = SlideTransition(true);
    static VerticalSlideTransition v_slide_transition = VerticalSlideTransition(false);
    static VerticalSlideTransition rev_v_slide_transition = VerticalSlideTransition(true);

    switch(type) {
        case TRANSITION_WIPE: return &wipe_transition;
        case TRANSITION_SLIDE_HORIZONTAL_LEFT:  return &h_slide_transition;
        case TRANSITION_SLIDE_HORIZONTAL_RIGHT:  return &rev_h_slide_transition;
        case TRANSITION_SLIDE_VERTICAL_UP: return &v_slide_transition;
        case TRANSITION_SLIDE_VERTICAL_DOWN: return &rev_v_slide_transition;
        case TRANSITION_RANDOM: return transition_type_to_transition( (transition_type_t) ((esp_random() % (TRANSITION_RANDOM - 1)) + 1));

        case TRANSITION_NONE:
        default:
            return nullptr;
    }
}