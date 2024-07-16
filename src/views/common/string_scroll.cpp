#include "views/common/string_scroll.h"
#include <service/prefs.h>

StringScroll::StringScroll(const font_definition_t * f, const char * s) {
    font = f;
    string = s;
    position = INT_MAX;
    string_width = 0;
    scroll_only_if_not_fit = true;
    start_at_visible = false;
    align_to_right = false;
    y_position = 0;
    wait_frames = 0;
    holdoff = 0;

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
    prepare();
}

void StringScroll::prepare() {
    position = INT_MAX;
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
            fb->put_string(font, string, align_to_right ? fb->get_width() - string_width : 0, y_position);
            return;
        }
    }

    if(position == INT_MAX) {
        if(start_at_visible) {
            position = fb->get_width();
            wait_frames = holdoff;
        } else {
            position = 0;
        }
    }

    frame_counter++;
    if(frame_counter % frame_divisor == 0) {
        if(wait_frames == 0) {
            position += increment;
            if(position >= fb->get_width() + string_width) {
                position = 0;
            } else if(position == fb->get_width()) {
                wait_frames = holdoff;
            }
        } else {
            wait_frames--;
        }
    }

    fb->put_string(font, string, fb->get_width() - position, y_position);
}