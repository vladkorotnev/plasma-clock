#include "views/transitions/wipe.h"


void WipeTransition::render(FantaManipulator * screen) {
    render_backing_views();
    framecount += 1;

    if(framecount >= screen->get_width()) {
        screen->put_fanta(backingB->get_fanta(), 0, 0, backingB->get_width(), backingB->get_height());
        return;
    }

    screen->put_fanta(backingA->get_fanta(), 0, 0, screen->get_width() - framecount - 1, backingA->get_height());
    FantaManipulator * rightSide = backingB->slice(backingB->get_width() - framecount, framecount - 1);
    screen->put_fanta(rightSide->get_fanta(), screen->get_width() - framecount, 0, rightSide->get_width(), rightSide->get_height());
    delete rightSide;
}

bool WipeTransition::is_complete() {
    return framecount >= backingA->get_width();
}