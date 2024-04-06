#pragma once
#include <plasma/framebuffer.h>
#include <sound/beeper.h>

class SimpleClock {
public:
    SimpleClock(FantaManipulator*, Beeper*);
    void render();
private:
    void draw_dropping_digit(char, char, int, int);
    void draw_dropping_number(int, int, int, int);
    FantaManipulator * framebuffer;
    Beeper * beeper;
    const font_definition_t * font;
};