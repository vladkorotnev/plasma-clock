#pragma once
#include "../transition_animator.h"
#include <graphics/framebuffer.h>

class WipeTransition: public Transition {
public:
    WipeTransition();
    bool render(FantaManipulator*, TransitionAnimationCoordinator*);
    void rewind();

private:
    uint8_t framecount;
};