#include <views/common/dropping_digits.h>


inline void itoa_padded(uint i, char * a) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
    a[2] = 0;
}

void DroppingDigits::draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset) {
    if(phase <= 0 || current == next) {
        framebuffer->put_glyph(font, current, left_offset, 0);
    } else if (phase >= 16) {
        framebuffer->put_glyph(font, next, left_offset, 0);
    } else {
        framebuffer->put_glyph(font, current, left_offset, phase);
        framebuffer->put_glyph(font, next, left_offset, phase - 16);
    }
}

void DroppingDigits::draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset) {
    char buf[5]; // Allow for 4-digit numbers
    char buf_next[5]; // Allow for 4-digit numbers

    // Handle 4-digit numbers
    if (current >= 1000 || next >= 1000) {
        if (phase <= 0) {
            snprintf(buf, 5, "%04d", current);
            draw_dropping_digit(fb, buf[0], buf[0], 0, left_offset);
            draw_dropping_digit(fb, buf[1], buf[1], 0, left_offset + font->width);
            draw_dropping_digit(fb, buf[2], buf[2], 0, left_offset + 2 * font->width);
            draw_dropping_digit(fb, buf[3], buf[3], 0, left_offset + 3 * font->width);
        } else if (phase >= 16) {
            snprintf(buf, 5, "%04d", next);
            draw_dropping_digit(fb, buf[0], buf[0], 0, left_offset);
            draw_dropping_digit(fb, buf[1], buf[1], 0, left_offset + font->width);
            draw_dropping_digit(fb, buf[2], buf[2], 0, left_offset + 2 * font->width);
            draw_dropping_digit(fb, buf[3], buf[3], 0, left_offset + 3 * font->width);
        } else {
            snprintf(buf, 5, "%04d", current);
            snprintf(buf_next, 5, "%04d", next);
            draw_dropping_digit(fb, buf[0], buf_next[0], phase, left_offset);
            draw_dropping_digit(fb, buf[1], buf_next[1], phase, left_offset + font->width);
            draw_dropping_digit(fb, buf[2], buf_next[2], phase, left_offset + 2 * font->width);
            draw_dropping_digit(fb, buf[3], buf_next[3], phase, left_offset + 3 * font->width);
        }
    } else {
        // Handle 2-digit numbers (original code)
        if (phase <= 0) {
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
}
