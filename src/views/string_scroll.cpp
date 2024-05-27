#include "views/string_scroll.h"
#include <service/prefs.h>

StringScroll::StringScroll(const font_definition_t * f) {
    font = f;
    string = nullptr;
    position = 0;
    string_width = 0;

    switch(prefs_get_int(PREFS_KEY_DISP_SCROLL_SPEED)) {
        case 0:
            // Slow
            frame_divisor = 3;
            increment = 1;
            break;
        case 1:
            // Medium
            frame_divisor = 2;
            increment = 1;
            break;
        case 2:
            // Fast
            frame_divisor = 1;
            increment = 1;
            break;
        case 3:
            // Sonic
            frame_divisor = 2;
            increment = 2;
            break;

        default:
            frame_divisor = 2;
            increment = 1;
            break;
    }
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
        position += increment;
        if(position >= fb->get_width() + string_width) {
            position = 0;
        }
    }

    fb->put_string(font, string, fb->get_width() - position, y_position);
}