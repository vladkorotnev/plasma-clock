#include <views/simple_clock.h>
#include <service/time.h>
#include <fonts.h>
#include <esp32-hal-ledc.h>

#define CLOCK_SEPARATOR ':'
#define CLOCK_SEPARATOR_OFF ' '

static const int EASING_CURVE[32] = { 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16 };

SimpleClock::SimpleClock(FantaManipulator * fb) {
    framebuffer = fb;
    font = &xnu_font;

    int char_count = 8; // XX:XX:XX
    int text_width = char_count * font->width;
    int left_offset = fb->get_width() - text_width;// / 2 - text_width / 2;
    framebuffer = fb->slice(left_offset, text_width);
}

void itoa_padded(int i, char * a) {
    itoa(i, a, 10);
    if(i < 10) {
        a[2] = 0;
        a[1] = a[0];
        a[0] = '0';
    }
}

void SimpleClock::draw_dropping_digit(char current, char next, int phase, int left_offset) {
    if(phase <= 0 || current == next) {
        framebuffer->put_glyph(font, current, left_offset, 0);
    } else if (phase >= 16) {
        framebuffer->put_glyph(font, next, left_offset, 0);
    } else {
        framebuffer->put_glyph(font, current, left_offset, phase);
        framebuffer->put_glyph(font, next, left_offset, phase - 16);
    }
}

void SimpleClock::draw_dropping_number(int current, int next, int phase, int left_offset) {
    char buf[3];
    char buf_next[3];
    
    if(phase <= 0) {
        itoa_padded(current, buf);
        draw_dropping_digit(buf[0], buf[0], 0, left_offset);
        draw_dropping_digit(buf[1], buf[1], 0, left_offset + font->width);
    } else if (phase >= 16) {
        itoa_padded(next, buf);
        draw_dropping_digit(buf[0], buf[0], 0, left_offset);
        draw_dropping_digit(buf[1], buf[1], 0, left_offset + font->width);
    } else {
        itoa_padded(current, buf);
        itoa_padded(next, buf_next);
        draw_dropping_digit(buf[0], buf_next[0], phase, left_offset);
        draw_dropping_digit(buf[1], buf_next[1], phase, left_offset + font->width);
    }
}


void SimpleClock::render() {
    int left_offset = 0;

    tk_time_of_day_t now = get_current_time_precise();
    
    tk_time_of_day_t next_time = now;
    next_time.millisecond = 0;
    next_time.second += 1;
    if(next_time.second == 60) {
        next_time.second = 0;
        next_time.minute += 1;
    }
    if(next_time.minute == 60) {
        next_time.hour += 1;
        next_time.minute = 0;
    }
    if(next_time.hour == 24) {
        next_time.hour = 0;
    }

    int remain_ms = (1000 - now.millisecond);
    const int ms_per_step = 15;
    const int steps = 32;
    int phase = 0;
    char separator = (remain_ms < 490) ? CLOCK_SEPARATOR_OFF : CLOCK_SEPARATOR;

    if(remain_ms <= steps * ms_per_step) {
        phase = 32 - (remain_ms / ms_per_step);
    }

    phase = EASING_CURVE[phase];

    draw_dropping_number(now.hour, next_time.hour, phase, left_offset);
    left_offset += 2 * font->width;
    
    framebuffer->put_glyph(font, separator, left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(now.minute, next_time.minute, phase, left_offset);
    left_offset += 2 * font->width;
    
    framebuffer->put_glyph(font, separator, left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(now.second, next_time.second, phase, left_offset);
}