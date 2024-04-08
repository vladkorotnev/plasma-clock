#include "views/transitions/slide.h"

SlideTransition::SlideTransition() {
    framecount = 0;
}

bool SlideTransition::render(FantaManipulator * screen, TransitionAnimationCoordinator* coord) {
    framecount += 1;
    FantaManipulator* backingB = coord->backingB;
    FantaManipulator* backingA = coord->backingA;

    if(framecount >= screen->get_width()) {
        screen->put_fanta(backingB->get_fanta(), 0, 0, backingB->get_width(), backingB->get_height());
        return true;
    }

    screen->put_fanta(backingA->get_fanta(), -framecount, 0, backingA->get_width(), backingA->get_height());
    screen->put_fanta(backingB->get_fanta(), screen->get_width() - framecount, 0, backingB->get_width(), backingB->get_height());

    return false;
}

void SlideTransition::rewind() {
    framecount = 0;
}