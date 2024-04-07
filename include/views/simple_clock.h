#pragma once
#include <plasma/framebuffer.h>
#include <sound/beeper.h>
#include <service/time.h>
#include "view.h"

class SimpleClock: public Renderable {
public:
    SimpleClock(FantaManipulator*, Beeper*);
    void render();
    void step();
private:
    void draw_dropping_digit(char, char, int, int);
    void draw_dropping_number(int, int, int, int);
    FantaManipulator * framebuffer;
    Beeper * beeper;
    const font_definition_t * font;
    bool tick;
    tk_time_of_day_t now;
    tk_time_of_day_t next_time;
    int phase;
    char separator;
};