#pragma once
#include "../transition_animator.h"
#include <plasma/framebuffer.h>

class WipeTransition: public Transition {
public:
    WipeTransition();
    bool render(FantaManipulator*, TransitionAnimationCoordinator*);
    void rewind();

private:
    uint8_t framecount;
};