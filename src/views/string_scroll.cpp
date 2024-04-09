#include "views/string_scroll.h"

StringScroll::StringScroll(const font_definition_t * f) {
    font = f;
    string = nullptr;
    position = 0;
    frame_divisor = 2;
    string_width = 0;
}

void StringScroll::set_string(const char * s) {
    string = s;
    position = 0;
    frame_counter = 0;
    if(string == nullptr) return;
    string_width = measure_string_width(font, string);
}

void StringScroll::set_y_position(int y) {
    y_position = y;
}

void StringScroll::render(FantaManipulator * fb) {
    if(string == nullptr) return;

    frame_counter++;
    if(frame_counter % frame_divisor == 0) {
        position++;
        if(position >= fb->get_width() + string_width) {
            position = 0;
        }
    }

    fb->put_string(font, string, fb->get_width() - position, y_position);
}