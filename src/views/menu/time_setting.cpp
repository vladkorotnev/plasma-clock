#include <views/menu/time_setting.h>
#include <input/keys.h>

inline void itoa_padded(uint i, char * a) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
    a[2] = 0;
}

inline void MenuTimeSettingView::draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset) {
    if(phase <= 0 || current == next) {
        framebuffer->put_glyph(font, current, left_offset, 0);
    } else if (phase >= 16) {
        framebuffer->put_glyph(font, next, left_offset, 0);
    } else {
        framebuffer->put_glyph(font, current, left_offset, phase);
        framebuffer->put_glyph(font, next, left_offset, phase - 16);
    }
}

inline void MenuTimeSettingView::draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset) {
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

void MenuTimeSettingView::render(FantaManipulator *fb) {
    if(animationPhase > -1) {
        animationPhase++;
        cursorTimer = 0;
        isShowingCursor = false;
        if(animationPhase == 16) {
            animationPhase = -1;
            isShowingCursor = true;
            prevHour = hour;
            prevMinute = minute;
        }
    } else {
        prevHour = hour;
        prevMinute = minute;
    }

    cursorTimer++;
    if(cursorTimer == 30) {
        cursorTimer = 0;
        isShowingCursor = !isShowingCursor;
    }

    int char_count = 5; // XX:XX
    int text_width = char_count * font->width;
    int left_offset = fb->get_width() / 2 - text_width / 2;
    int cursor_offset = 0;

    draw_dropping_number(fb, prevHour, hour, animationPhase, left_offset);
    if(!isEditingMinutes) cursor_offset = left_offset;
    left_offset += 2 * font->width;
    
    fb->put_glyph(font, ':', left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(fb, prevMinute, minute, animationPhase, left_offset);
    if(isEditingMinutes) cursor_offset = left_offset;

    if(isShowingCursor) {
        fb->rect(cursor_offset - 2, 0, cursor_offset + 2 * font->width + 1, 15, false);
    }
}

void MenuTimeSettingView::add_hr(bool increment) {
    if(increment) {
        hour += 1;
        if(hour > 23) hour = 0;
    } else {
        hour -= 1;
        if(hour < 0) hour = 23;
    }

    if(animationPhase == -1) animationPhase = 0;
}

void MenuTimeSettingView::add_min(bool increment) {
    if(increment) {
        minute += 1;
        if(minute > 59) {
            minute = 0;
            add_hr(true);
        }
    } else {
        minute -= 1;
        if(minute < 0) {
            minute = 59;
            add_hr(false);
        }
    }
    if(animationPhase == -1) animationPhase = 0;
}

void MenuTimeSettingView::step() {
    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        onFinish(hour, minute);
    } 
    else if(hid_test_key_state_repetition(KEY_DOWN)) {
        if(isEditingMinutes) add_min(false);
        else add_hr(false);
    }
    else if(hid_test_key_state_repetition(KEY_UP)) {
        if(isEditingMinutes) add_min(true);
        else add_hr(true);
    }
    else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
        isEditingMinutes = !isEditingMinutes;
    }

    if(animationPhase == 8) {
        beeper->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
    }
}