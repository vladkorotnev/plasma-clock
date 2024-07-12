#include "views/common/string_scroll.h"
#include <service/prefs.h>

StringScroll::StringScroll(const font_definition_t * f) {
    font = f;
    string = nullptr;
    position = 0;
    string_width = 0;
    scroll_only_if_not_fit = true;

    switch(prefs_get_int(PREFS_KEY_DISP_SCROLL_SPEED)) {
        case SCROLL_SPEED_SLOW:
            // Slow
            frame_divisor = 3;
            increment = 1;
            break;
        case SCROLL_SPEED_NORMAL:
            // Medium
            frame_divisor = 2;
            increment = 1;
            break;
        case SCROLL_SPEED_FAST:
            // Fast
            frame_divisor = 1;
            increment = 1;
            break;
        case SCROLL_SPEED_SONIC:
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

int StringScroll::estimated_frame_count() {
    return string_width * frame_divisor / increment;
}

void StringScroll::render(FantaManipulator * fb) {
    if(string == nullptr) return;

    if(scroll_only_if_not_fit) {
        if(string_width <= fb->get_width()) {
            fb->put_string(font, string, 0, y_position);
            return;
        }
    }

    frame_counter++;
    if(frame_counter % frame_divisor == 0) {
        position += increment;
        if(position >= fb->get_width() + string_width) {
            position = 0;
        }
    }

    fb->put_string(font, string, fb->get_width() - position, y_position);
}