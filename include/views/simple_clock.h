#pragma once
#include <plasma/framebuffer.h>
#include <sound/beeper.h>
#include <service/time.h>
#include "view.h"

class SimpleClock: public Renderable {
public:
    SimpleClock(Beeper*);
    void render(FantaManipulator*);
    void step();
private:
    void draw_dropping_digit(FantaManipulator *, char, char, int, int);
    void draw_dropping_number(FantaManipulator *, int, int, int, int);
    Beeper * beeper;
    const font_definition_t * font;
    bool tick;
    tk_time_of_day_t now;
    tk_time_of_day_t next_time;
    int phase;
    char separator;
};