#pragma once
#include <views/framework.h>
#include <fonts.h>

class DroppingDigits {
protected:
    DroppingDigits() : font(&xnu_font) { }
    void draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset);
    void draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset);
    const font_definition_t * font;
};