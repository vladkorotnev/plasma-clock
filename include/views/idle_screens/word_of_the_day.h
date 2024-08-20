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
    int desired_display_time();

private:
    const font_definition_t * font;
    char word_buffer[32];
    char definition_buffer[256];
    TickType_t last_update;
    AniSpriteView * animation;
    StringScroll * top_line;
    StringScroll * bottom_line;
};

#endif