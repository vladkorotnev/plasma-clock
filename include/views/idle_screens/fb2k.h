#pragma once
#include <views/common/view.h>
#include <graphics/fanta_manipulator.h>
#include <views/common/string_scroll.h>

class Fb2kView: public Screen {
public:
    Fb2kView();
    void prepare();
    void step();
    void render(FantaManipulator*);
    int desired_display_time();

private:
    void update_if_needed();
    const font_definition_t * font;
    char artist_buffer[128];
    char title_buffer[128];
    TickType_t last_update;
    StringScroll * top_line;
    StringScroll * bottom_line;
};