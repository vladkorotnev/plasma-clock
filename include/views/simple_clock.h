#pragma once
#include <plasma/framebuffer.h>
#include <sound/beeper.h>

class SimpleClock {
public:
    SimpleClock(FantaManipulator*);
    void render();
private:
    void draw_dropping_digit(char, char, int, int);
    void draw_dropping_number(int, int, int, int);
    FantaManipulator * framebuffer;
    const font_definition_t * font;
};