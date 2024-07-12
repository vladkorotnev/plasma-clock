#pragma once
#include <views/framework.h>

/// @brief Scrolling string view
class StringScroll: public Renderable {
public:
    StringScroll(const font_definition_t*, const char* string = nullptr);
    void set_string(const char*);
    void set_y_position(int);
    int estimated_frame_count();
    void prepare();
    void render(FantaManipulator *);

private:
    const font_definition_t * font;
    const char * string;
    bool scroll_only_if_not_fit;
    bool start_at_visible;
    int y_position;
    int position;
    int frame_divisor;
    int increment;
    int string_width;
    int frame_counter;
};