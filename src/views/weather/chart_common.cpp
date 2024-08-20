#include "views/weather/chart_common.h"
#include <fonts.h>
#include <vector>
#include <algorithm>

void WeatherChartCommon::prepare() {
    Composable::prepare();
    reveal_index = 0;
    hint_framecounter = 0;
    show_hint = true;
    update_minmax_if_needed();
}

void WeatherChartCommon::render(FantaManipulator* fb) {
    fb->clear();
    static char buf[8] = { 0 };

    Composable::render(fb);
    if(points.size() > 0) {
        for(int p_idx = 0; p_idx < std::min((int)points.size(), reveal_index); p_idx++) {
            auto p = points[p_idx];

            int y = fb->get_height() - (p.value - minimum) * fb->get_height() / (maximum - minimum);
            if(filled) {
                fb->line(p_idx, y, p_idx, fb->get_height(), (p_idx == cursor_index) ? cursor_phase : true);
            } else {
                fb->plot_pixel(p_idx, y, (p_idx == cursor_index) ? cursor_phase : true);
            }
        }

        for(int p_idx = 0; p_idx < points.size(); p_idx++) {
            auto p = points[p_idx];
            int y = fb->get_height() - (p.value - minimum) * fb->get_height() / (maximum - minimum);

            if(p.annotation >= 0) {
                snprintf(buf, 7, "%d", p.annotation);
                int x = (p_idx >= 3) ? p_idx - 3 : 0; 
                int w = measure_string_width(&fps_counter_font, buf) + 1;
                int txt_y = fb->get_height() - 5;
                if(y >= txt_y) txt_y = y - 6;
                fb->rect(x, txt_y - 1, x + w, txt_y + 5, true, false);
                fb->put_string(&fps_counter_font, buf, x + 1, txt_y);
            }
        }
    }

    if(show_minimum) {
        snprintf(buf, 7, "%d", minimum);
        int w = measure_string_width(&fps_counter_font, buf) + 1;
        int x = fb->get_width() - w; 
        fb->rect(x, fb->get_height() - 5, x + w, fb->get_height(), true, false);
        fb->put_string(&fps_counter_font, buf, x + 1, fb->get_height() - 5);
    }
    
    if(show_maximum) {
        snprintf(buf, 7, "%d", maximum);
        int w = measure_string_width(&fps_counter_font, buf) + 1;
        int x = fb->get_width() - w; 
        fb->rect(x, 0, x + w, 6, true, false);
        fb->put_string(&fps_counter_font, buf, x + 1, 0);
    }

    if(show_hint && hint != nullptr) {
        fb->put_string(&keyrus0808_font, hint, 0, 0, true);
    }
}

void WeatherChartCommon::step() {
    Composable::step();
    if(reveal_index < points.size()) {
        reveal_index += 4;
    }
    cursor_framecounter++;
    if(cursor_framecounter == 31) {
        cursor_framecounter = 0;
        cursor_phase ^= 1;
    }

    if(hint_framecounter < 121) {
        hint_framecounter++;
        if(hint_framecounter == 120) {
            show_hint = false;
        }
    }
}

void WeatherChartCommon::update_minmax_if_needed() {
    if(!autoscale) return;

    int min_point = INT32_MAX;
    int max_point = INT32_MIN;

    for(auto p: points) {
        if(p.value < min_point) min_point = p.value;
        if(p.value > max_point) max_point = p.value;
    }

    minimum = min_point - 1;
    maximum = max_point + 1;
}
