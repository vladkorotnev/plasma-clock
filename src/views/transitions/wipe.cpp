#include "views/transitions/wipe.h"

WipeTransition::WipeTransition() {
    framecount = 0;
}

bool WipeTransition::render(FantaManipulator * screen, TransitionAnimationCoordinator* coord) {
    framecount += screen->get_width() / 16;
    FantaManipulator* backingB = coord->backingB;
    FantaManipulator* backingA = coord->backingA;

    if(framecount >= screen->get_width()) {
        screen->put_fanta(backingB->get_fanta(), 0, 0, backingB->get_width(), backingB->get_height());
        return true;
    }

    screen->put_fanta(backingA->get_fanta(), 0, 0, screen->get_width() - framecount - 1, backingA->get_height());
    FantaManipulator * rightSide = backingB->slice(backingB->get_width() - framecount, framecount - 1);
    screen->put_fanta(rightSide->get_fanta(), screen->get_width() - framecount, 0, rightSide->get_width(), rightSide->get_height());
    delete rightSide;

    return false;
}

void WipeTransition::rewind() {
    framecount = 0;
}