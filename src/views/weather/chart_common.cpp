#include "views/weather/chart_common.h"
#include <graphics/font.h>
#include <vector>
#include <algorithm>

WeatherChartCommon::WeatherChartCommon() {
    hint_lbl = new StringScroll(find_font(FONT_STYLE_UI_TEXT));
    hint_lbl->x_offset = 0;
    hint_lbl->set_y_position(1);
    hint_lbl->render_mode = TEXT_NO_BACKGROUND | TEXT_OUTLINED;
    add_composable(hint_lbl);
    legend_font = find_font(FONT_STYLE_HUD_DIGITS);
}

void WeatherChartCommon::prepare() {
    hint_lbl->set_string(hint);
    Screen::prepare();
    reveal_index = 0;
    hint_framecounter = 0;
    hint_lbl->hidden = false;
    show_legend = true;
    update_minmax_if_needed();
}

void WeatherChartCommon::render(FantaManipulator* fb) {
    fb->clear();
    static char buf[8] = { 0 };
    bool cursor_state = false;
#ifdef COMPOSABLE_NO_EVENODD
    cursor_state = cursor_phase;
#else
    cursor_state = cursor_phase | even_odd;
#endif

    if(points.size() > 0) {
        for(int p_idx = 0; p_idx < std::min((int)points.size(), reveal_index); p_idx++) {
            auto p = points[p_idx];

            int y = fb->get_height() - (p.value - minimum) * fb->get_height() / (maximum - minimum);
            if(filled) {
                fb->line(p_idx, y, p_idx, fb->get_height(), (p_idx == cursor_index) ? cursor_state : true);
            } else {
                fb->plot_pixel(p_idx, y, (p_idx == cursor_index) ? cursor_state : true);
            }
        }

        if(show_legend) {
            for(int p_idx = 0; p_idx < points.size(); p_idx++) {
                auto p = points[p_idx];
                int y = fb->get_height() - (p.value - minimum) * fb->get_height() / (maximum - minimum);

                if(p.annotation >= 0) {
                    snprintf(buf, 7, "%d", p.annotation);
                    int x = (p_idx >= 3) ? p_idx - 3 : 0; 
                    int w = measure_string_width(legend_font, buf) + 1;
                    int txt_y = fb->get_height() - 5;
                    if(y >= txt_y) txt_y = y - 6;
                    fb->rect(x, txt_y - 1, x + w, txt_y + 5, true, false);
                    fb->put_string(legend_font, buf, x + 1, txt_y, TEXT_NO_BACKGROUND | TEXT_OUTLINED);
                }
            }
        }
    }

    if(show_legend) {
        if(show_minimum) {
            snprintf(buf, 7, "%d", minimum);
            int w = measure_string_width(legend_font, buf) + 1;
            int x = fb->get_width() - w; 
            fb->put_string(legend_font, buf, x, fb->get_height() - 5, TEXT_NO_BACKGROUND | TEXT_OUTLINED);
        }
        
        if(show_maximum) {
            snprintf(buf, 7, "%d", maximum);
            int w = measure_string_width(legend_font, buf) + 1;
            int x = fb->get_width() - w; 
            fb->put_string(legend_font, buf, x, 0, TEXT_NO_BACKGROUND | TEXT_OUTLINED);
        }
    }

    Screen::render(fb);
}

void WeatherChartCommon::step() {
    Screen::step();
    if(reveal_index < points.size()) {
        reveal_index += 4;
    }
    cursor_framecounter++;
    if(cursor_framecounter == 31) {
        cursor_framecounter = 0;
        cursor_phase ^= 1;
    }

    if(hint_framecounter < 241) {
        hint_framecounter++;
        if(hint_framecounter == 120) {
            hint_lbl->hidden = true;
        }
        if(hint_framecounter == ((hint == nullptr) ? 120 : 240)) {
            show_legend = false;
        }
    }

    if(hid_test_key_any(KEY_HEADPAT | KEY_RIGHT) == KEYSTATE_HIT) {
        hint_framecounter = 0;
        hint_lbl->hidden = false;
        show_legend = true;
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
