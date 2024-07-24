#include "views/common/string_scroll.h"
#include <service/prefs.h>

StringScroll::StringScroll(const font_definition_t * f, const char * s): Composable() {
    font = f;
    position = INT_MAX;
    string_width = 0;
    scroll_only_if_not_fit = true;
    start_at_visible = false;
    align_to_right = false;
    y_position = 0;
    wait_frames = 0;
    left_margin = 0;
    holdoff = 0;
    backing_buffer = nullptr;
    stopped = false;
    set_string(s);

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
    if(backing_buffer != nullptr) {
        free(backing_buffer);
    }

    if(string != nullptr) {
        int width = measure_string_width(font, string);
        backing_buffer_width = width;
        string_width = width;

        backing_buffer_size = 2 * width;
        backing_buffer = (fanta_buffer_t) gralloc(backing_buffer_size);
        if(backing_buffer == nullptr) {
            ESP_LOGE("SCRL", "Out Of Memory allocating backing buffer of size %i", backing_buffer_size);
        }
        else {
            bool dummy;
            FantaManipulator * bb = new FantaManipulator(backing_buffer, backing_buffer_size, width, 16, NULL, &dummy);
            bb->put_string(font, string, 0, 0);
            delete bb;
        }
    }

    prepare();
}

void StringScroll::prepare() {
    position = INT_MAX;
    frame_counter = 0;
}

void StringScroll::set_y_position(int y) {
    y_position = y;
}

int StringScroll::estimated_frame_count() {
    return string_width * frame_divisor / increment;
}

void StringScroll::render(FantaManipulator * fb) {
    if(backing_buffer == nullptr) return;

    if(scroll_only_if_not_fit || stopped) {
        if(string_width <= fb->get_width() || stopped) {
            fb->put_fanta(backing_buffer, align_to_right ? fb->get_width() - string_width : 0, y_position, backing_buffer_width, font->height);
            return;
        }
    }

    if(position == INT_MAX) {
        if(start_at_visible) {
            position = fb->get_width() - left_margin;
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
            } else if(position == fb->get_width() - left_margin) {
                wait_frames = holdoff;
            }
        } else {
            wait_frames--;
        }
    }

    fb->put_fanta(backing_buffer, fb->get_width() - position, y_position, backing_buffer_width, font->height);
} 

void StringScroll::rewind() {
    position = 0;
}