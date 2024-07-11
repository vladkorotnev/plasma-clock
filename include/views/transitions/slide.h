#pragma once
#include "../transition_animator.h"
#include <graphics/framebuffer.h>

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
    bool is_reverse = false;
    VerticalSlideTransition(bool reverse = false);
    bool render(FantaManipulator*, TransitionAnimationCoordinator*);
    void rewind();

private:
    uint8_t framecount;
};