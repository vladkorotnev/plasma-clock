#pragma once
#include <device_config.h>

#if HAS(WORDNIK_API)

#include <views/common/view.h>
#include <graphics/fanta_manipulator.h>
#include <views/common/string_scroll.h>

class WordOfTheDayView: public Screen {
public:
    WordOfTheDayView();
    ~WordOfTheDayView();
    void prepare();
    void step();
    void render(FantaManipulator*);
    int desired_display_time();

private:
    const font_definition_t * font;
    char word_buffer[32];
    char definition_buffer[256];
    TickType_t last_update;
    ani_sprite_state_t icon_state;
    sprite_t current_icon_frame;
    StringScroll * bottom_line;
};

#endif