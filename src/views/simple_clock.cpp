#include <views/simple_clock.h>
#include <fonts.h>
#include <esp32-hal-ledc.h>

#define CLOCK_SEPARATOR ':'
#define CLOCK_SEPARATOR_OFF ' '

static const int EASING_CURVE[32] = { 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16 };

SimpleClock::SimpleClock() {
    font = &xnu_font;
    now = { 0 };
    next_time = { 0 };
    phase = 0;
    separator = CLOCK_SEPARATOR;
}

inline void itoa_padded(uint i, char * a) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
    a[2] = 0;
}

inline void SimpleClock::draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset) {
    if(phase <= 0 || current == next) {
        framebuffer->put_glyph(font, current, left_offset, 0);
    } else if (phase >= 16) {
        framebuffer->put_glyph(font, next, left_offset, 0);
    } else {
        framebuffer->put_glyph(font, current, left_offset, phase);
        framebuffer->put_glyph(font, next, left_offset, phase - 16);
    }
}

inline void SimpleClock::draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset) {
    char buf[3];
    char buf_next[3];
    
    if(phase <= 0) {
        itoa_padded(current, buf);
        draw_dropping_digit(fb, buf[0], buf[0], 0, left_offset);
        draw_dropping_digit(fb, buf[1], buf[1], 0, left_offset + font->width);
    } else if (phase >= 16) {
        itoa_padded(next, buf);
        draw_dropping_digit(fb, buf[0], buf[0], 0, left_offset);
        draw_dropping_digit(fb, buf[1], buf[1], 0, left_offset + font->width);
    } else {
        itoa_padded(current, buf);
        itoa_padded(next, buf_next);
        draw_dropping_digit(fb, buf[0], buf_next[0], phase, left_offset);
        draw_dropping_digit(fb, buf[1], buf_next[1], phase, left_offset + font->width);
    }
}

void add_one_second(tk_time_of_day_t * time) {
    time->millisecond = 0;
    time->second += 1;
    if(time->second == 60) {
        time->second = 0;
        time->minute += 1;
    }
    if(time->minute == 60) {
        time->hour += 1;
        time->minute = 0;
    }
    if(time->hour == 24) {
        time->hour = 0;
    }
}

void subtract_one_second(tk_time_of_day_t * time) {
    if(time->second == 0) {
        time->second = 59;
        if(time->minute == 0) {
            time->minute = 59;
            if(time->hour == 0) {
                time->hour = 23;
            } else {
                time->hour -= 1;
            }
        } else {
            time->minute -= 1;
        }
    } else {
        time->second -= 1;
    }
}

void SimpleClock::step() {
    now = get_current_time_precise();
    next_time = now;

    const int ms_per_step = 15;
    const int steps = 32;
    int remain_ms = (1000 - now.millisecond);

    phase = 0;

    // When animating precisely on-the-millisecond, the digit does an annoying blink of the last value sometimes
    // So animate the first half in the "previous" second, and the second half in the "next" second
    if(remain_ms < (steps/2) * ms_per_step) {
        phase = (steps/2) - (remain_ms / ms_per_step);
        add_one_second(&next_time);
    } else if (now.millisecond < (steps/2) * ms_per_step) {
        phase = (steps/2) + (now.millisecond / ms_per_step);
        subtract_one_second(&now);
    }
    phase = EASING_CURVE[phase];

    separator = (phase == 0) ? CLOCK_SEPARATOR_OFF : CLOCK_SEPARATOR;
}

void SimpleClock::render(FantaManipulator *framebuffer) {
    int char_count = 8; // XX:XX:XX
    int text_width = char_count * font->width;
    int left_offset = framebuffer->get_width() / 2 - text_width / 2;

    draw_dropping_number(framebuffer, now.hour, next_time.hour, phase, left_offset);
    left_offset += 2 * font->width;
    
    framebuffer->put_glyph(font, separator, left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(framebuffer, now.minute, next_time.minute, phase, left_offset);
    left_offset += 2 * font->width;
    
    framebuffer->put_glyph(font, separator, left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(framebuffer, now.second, next_time.second, phase, left_offset);
}
