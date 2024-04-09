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

class VerticalSlideTransition: public Transition {
public:
    VerticalSlideTransition();
    bool render(FantaManipulator*, TransitionAnimationCoordinator*);
    void rewind();

private:
    uint8_t framecount;
};