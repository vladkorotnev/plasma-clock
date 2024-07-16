#pragma once
#include <views/framework.h>

/// @brief Scrolling string view
class StringScroll: public Renderable {
public:
    StringScroll(const font_definition_t*, const char* string = nullptr);
    int string_width;
    bool scroll_only_if_not_fit;
    bool align_to_right;
    bool start_at_visible;
    int holdoff;
    int left_margin;
    void set_string(const char*);
    void set_y_position(int);
    int estimated_frame_count();
    void prepare();
    void render(FantaManipulator *);

private:
    const font_definition_t * font;
    const char * string;
    int y_position;
    int position;
    int frame_divisor;
    int increment;
    int frame_counter;
    int wait_frames;
};