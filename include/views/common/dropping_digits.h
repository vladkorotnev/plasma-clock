#pragma once
#include <views/framework.h>
#include <graphics/font.h>

class DroppingDigits {
protected:
    DroppingDigits() : font(find_font(FONT_STYLE_CLOCK_FACE)) { }
    void draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset);
    void draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset);
    const font_definition_t * font;
};

class DroppingDigitView: public Composable, DroppingDigits {
public:
    int value;
    bool sound;
    DroppingDigitView(unsigned int digits, int initialValue, Beeper * b = nullptr):
        DroppingDigits(),
        value(initialValue),
        digits(digits),
        beeper(b),
        disp_value { value },
        phase { 0 },
        dir { true },
        sound { true } {
            snprintf(fmt, 16, "%%0%dd", digits);
            snprintf(buf_old, 16, fmt, disp_value);
            snprintf(buf_new, 16, fmt, value);
            width = digits * font->width;
        }

    void prepare() {
        disp_value = value;
        snprintf(buf_old, 16, fmt, value);
        snprintf(buf_new, 16, fmt, disp_value);
    }

    void render(FantaManipulator *fb) {
        if(phase > 0) {
            if(dir) {
                phase++;
            } else {
                phase--;
            }
            if(phase == 16 || phase == 0) {
                phase = 0;
                disp_value = value;
                snprintf(buf_old, 16, fmt, disp_value);
            }
        }

        int left_offset = 0;
        for(int i = 0; i < digits; i++) {
            draw_dropping_digit(fb, buf_old[i], buf_new[i], phase, left_offset);
            left_offset += font->width;
        }
    }

    void step() {
        if(disp_value != value && phase == 0) {
            if(disp_value - value == 1 || value - disp_value > 1) {
                // scroll backward
                phase = 16;
                snprintf(buf_old, 16, fmt, value);
                snprintf(buf_new, 16, fmt, disp_value);
                dir = false;
            } else  {
                // scroll forward
                phase = 1;
                snprintf(buf_old, 16, fmt, disp_value);
                snprintf(buf_new, 16, fmt, value);
                dir = true;
            }
        }

        if(phase == 8 && beeper && sound) {
            beeper->beep(CHANNEL_AMBIANCE, 100, 10);
        }
    }

private:
    char buf_old[16];
    char buf_new[16];
    char fmt[16];
    int disp_value;
    int digits;
    int phase;
    int wrapping_point;
    bool dir;
    Beeper * beeper;
};