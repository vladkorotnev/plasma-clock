#pragma once
#include <plasma/framebuffer.h>

class SimpleClock {
public:
    SimpleClock(PlasmaDisplayFramebuffer*);
    void render();
private:
    void draw_dropping_digit(char, char, int, int);
    void draw_dropping_number(int, int, int, int);
    PlasmaDisplayFramebuffer * framebuffer;
    const font_definition_t * font;
};