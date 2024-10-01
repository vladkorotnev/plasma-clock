#include <views/idle_screens/simple_clock.h>
#include <service/prefs.h>
#include <fonts.h>
#include <esp32-hal-ledc.h>

#define CLOCK_SEPARATOR ':'
#define CLOCK_SEPARATOR_OFF ' '

static const int EASING_CURVE[32] = { 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12, 13, 14, 15, 16 };

class SimpleClock::AmPmLabel: public Composite {
public:
    AmPmLabel() {}
    bool is_pm = false;

    void render(FantaManipulator *fb) override {
        Composite::render(fb);
        fb->put_sprite(is_pm ? &spr_p : &spr_a, 0, is_pm ? 7 : 2);
        fb->put_sprite(&spr_m, spr_a.width, is_pm ? 7 : 2);
    }

private:
    const uint8_t letter_a_data[5] = {
        0b0111,
        0b1001,
        0b1111,
        0b1001,
        0b1001,
    };
    const uint8_t letter_p_data[5] = {
        0b1110,
        0b1001,
        0b1110,
        0b1000,
        0b1000,
    };
    const uint8_t letter_m_data[5] = {
        0b1111,
        0b1011,
        0b1001,
        0b1001,
        0b1001,
    };
    const sprite_t spr_a = {
        .width = 5,
        .height = 5,
        .data = letter_a_data,
        .mask = nullptr,
        .format = SPRFMT_HORIZONTAL
    };
    const sprite_t spr_p = {
        .width = 5,
        .height = 5,
        .data = letter_p_data,
        .mask = nullptr,
        .format = SPRFMT_HORIZONTAL
    };
    const sprite_t spr_m = {
        .width = 5,
        .height = 5,
        .data = letter_m_data,
        .mask = nullptr,
        .format = SPRFMT_HORIZONTAL
    };

};

SimpleClock::SimpleClock(): DroppingDigits() {
    now = { 0 };
    next_time = { 0 };
    phase = 0;
    separator = CLOCK_SEPARATOR;
    pm_label = new AmPmLabel();
    pm_label->hidden = true;
    add_composable(pm_label);
    blink_separator = prefs_get_bool(PREFS_KEY_BLINK_SEPARATORS);
    show_seconds = prefs_get_bool(PREFS_KEY_SHOW_SECONDS);
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

    if(!prefs_get_bool(PREFS_KEY_DISP_24_HRS)) {
        bool now_pm = false;
        bool next_pm = false;
        convert_to_12h(&now,&now_pm);
        convert_to_12h(&next_time, &next_pm);

        pm_label->hidden = false;
        pm_label->gray = (now_pm != next_pm);
        pm_label->is_pm = now_pm;
    } else {
        pm_label->hidden = true;
    }

    blink_separator = prefs_get_bool(PREFS_KEY_BLINK_SEPARATORS);
    show_seconds = prefs_get_bool(PREFS_KEY_SHOW_SECONDS);
    separator = (blink_separator && phase != 0) ? CLOCK_SEPARATOR_OFF : CLOCK_SEPARATOR;
    Screen::step();
}

void SimpleClock::render(FantaManipulator *framebuffer) {
    int char_count = show_seconds ? 8 : 5; // XX:XX:XX
    int text_width = char_count * font->width;
    int left_offset = framebuffer->get_width() / 2 - text_width / 2;
    pm_label->x_offset = left_offset - 11;

    draw_dropping_number(framebuffer, now.hour, next_time.hour, phase, left_offset);
    left_offset += 2 * font->width;
    
    framebuffer->put_glyph(font, separator, left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(framebuffer, now.minute, next_time.minute, phase, left_offset);
    left_offset += 2 * font->width;
    
    if(show_seconds) {
        framebuffer->put_glyph(font, separator, left_offset, 0);
        left_offset += font->width;
        draw_dropping_number(framebuffer, now.second, next_time.second, phase, left_offset);
    }

    Screen::render(framebuffer);
}
