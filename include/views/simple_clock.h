#pragma once
#include <plasma/framebuffer.h>
#include <sound/beeper.h>
#include <service/time.h>
#include "view.h"

/// @brief A simple clock view
class SimpleClock: public Screen {
public:
    SimpleClock();
    void render(FantaManipulator*);
    void step();
private:
    void draw_dropping_digit(FantaManipulator *, char, char, int, int);
    void draw_dropping_number(FantaManipulator *, int, int, int, int);
    const font_definition_t * font;
    tk_time_of_day_t now;
    tk_time_of_day_t next_time;
    int phase;
    char separator;
    bool blink_separator;
};