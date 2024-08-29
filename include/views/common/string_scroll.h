#pragma once
#include <views/framework.h>

/// @brief Scrolling string view
class StringScroll: public Composable {
public:
    StringScroll(const font_definition_t*, const char* string = nullptr);
    int string_width;
    text_attributes_t render_mode = TEXT_NORMAL;
    bool scroll_only_if_not_fit;
    bool stopped;
    bool align_to_right;
    bool start_at_visible;
    int holdoff;
    int left_margin;
    void set_string(const char*);
    void set_y_position(int);
    void rewind();
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
    int v_padding;

    fanta_buffer_t backing_buffer;
    fanta_buffer_t mask_buffer;
    int backing_buffer_width;
    size_t backing_buffer_size;
};