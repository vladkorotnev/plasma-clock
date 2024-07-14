#include "views/transitions/slide.h"

SlideTransition::SlideTransition(bool reverse) {
    framecount = 0;
    is_reverse = reverse;
}

bool SlideTransition::render(FantaManipulator * screen, TransitionAnimationCoordinator* coord) {
    framecount += screen->get_width() / screen->get_height(); // <- to keep same speed as vertical, which is 1px per frame
    FantaManipulator* backingB = coord->backingB;
    FantaManipulator* backingA = coord->backingA;

    if(framecount >= screen->get_width()) {
        screen->put_fanta(backingB->get_fanta(), 0, 0, backingB->get_width(), backingB->get_height());
        return true;
    }

    if(is_reverse) {
        screen->put_fanta(backingA->get_fanta(), framecount, 0, backingA->get_width(), backingA->get_height());
        screen->put_fanta(backingB->get_fanta(),  -backingB->get_width() + framecount, 0, backingB->get_width(), backingB->get_height());
    } else {
        screen->put_fanta(backingA->get_fanta(), -framecount, 0, backingA->get_width(), backingA->get_height());
        screen->put_fanta(backingB->get_fanta(), backingB->get_width() - framecount, 0, backingB->get_width(), backingB->get_height());
    }

    return false;
}

void SlideTransition::rewind() {
    framecount = 0;
}

VerticalSlideTransition::VerticalSlideTransition(bool reverse) {
    framecount = 0;
    is_reverse = reverse;
}

bool VerticalSlideTransition::render(FantaManipulator * screen, TransitionAnimationCoordinator* coord) {
    framecount += 1;
    FantaManipulator* backingB = coord->backingB;
    FantaManipulator* backingA = coord->backingA;

    if(framecount >= screen->get_height()) {
        screen->put_fanta(backingB->get_fanta(), 0, 0, backingB->get_width(), backingB->get_height());
        return true;
    }

    if(is_reverse) {
        screen->put_fanta(backingA->get_fanta(), 0, framecount, backingA->get_width(), backingA->get_height());
        screen->put_fanta(backingB->get_fanta(), 0, -backingB->get_height() + framecount, backingB->get_width(), backingB->get_height());
    } else {
        screen->put_fanta(backingA->get_fanta(), 0, -framecount, backingA->get_width(), backingA->get_height());
        screen->put_fanta(backingB->get_fanta(), 0, backingB->get_height() - framecount, backingB->get_width(), backingB->get_height());
    }

    return false;
}

void VerticalSlideTransition::rewind() {
    framecount = 0;
}