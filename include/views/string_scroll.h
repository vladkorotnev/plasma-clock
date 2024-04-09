#pragma once
#include <views/framework.h>

class StringScroll: public Renderable {
public:
    StringScroll(const font_definition_t*);
    void set_string(const char*);
    void set_y_position(int);
    void render(FantaManipulator *);

private:
    const font_definition_t * font;
    const char * string;
    int y_position;
    int position;
    int frame_divisor;
    int string_width;
    int frame_counter;
};