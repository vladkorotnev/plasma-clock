#pragma once
#include "../transition_animator.h"
#include <plasma/framebuffer.h>

class WipeTransition: public TransitionAnimator {
public:
    using TransitionAnimator::TransitionAnimator;
    void render(FantaManipulator*);
    bool is_complete();

private:
    uint8_t framecount = 0;
};