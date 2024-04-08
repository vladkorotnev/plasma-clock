#pragma once
#include "transitions/wipe.h"
#include "transitions/slide.h"

typedef enum transition_type {
    TRANSITION_NONE,
    TRANSITION_WIPE,
    TRANSITION_SLIDE,
} transition_type_t;


static Transition* transition_type_to_transition(transition_type_t type) {  
    static WipeTransition wipe_transition = WipeTransition();
    static SlideTransition slide_transition = SlideTransition();

    switch(type) {
        case TRANSITION_WIPE:
            return &wipe_transition;
        case TRANSITION_SLIDE:
            return &slide_transition;
        case TRANSITION_NONE:
        default:
            return nullptr;
    }
}