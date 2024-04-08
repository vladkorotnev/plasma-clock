#pragma once
#include "../transition_animator.h"
#include <plasma/framebuffer.h>

class SlideTransition: public Transition {
public:
    SlideTransition();
    bool render(FantaManipulator*, TransitionAnimationCoordinator*);
    void rewind();

private:
    uint8_t framecount;
};