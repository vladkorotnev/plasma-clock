#include <views/menu/date_setting.h>
#include <input/keys.h>

inline void itoa_padded(uint i, char * a) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
    a[2] = 0;
}

inline void MenuDateSettingView::draw_dropping_digit(FantaManipulator *framebuffer, char current, char next, int phase, int left_offset) {
    if(phase <= 0 || current == next) {
        framebuffer->put_glyph(font, current, left_offset, 0);
    } else if (phase >= 16) {
        framebuffer->put_glyph(font, next, left_offset, 0);
    } else {
        framebuffer->put_glyph(font, current, left_offset, phase);
        framebuffer->put_glyph(font, next, left_offset, phase - 16);
    }
}

inline void MenuDateSettingView::draw_dropping_number(FantaManipulator *fb, int current, int next, int phase, int left_offset) {
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


void MenuDateSettingView::render(FantaManipulator *fb) {
    if(animationPhase > -1) {
        animationPhase++;
        cursorTimer = 0;
        isShowingCursor = false;
        if(animationPhase == 16) {
            animationPhase = -1;
            isShowingCursor = true;
            prevYear = year;
            prevMonth = month;
            prevDay = day;
        }
    } else {
        prevYear = year;
        prevMonth = month;
        prevDay = day;
    }

    cursorTimer++;
    if(cursorTimer == 30) {
        cursorTimer = 0;
        isShowingCursor = !isShowingCursor;
    }

    int char_count = 10; // YYYY/MM/dd
    int text_width = char_count * font->width;
    int left_offset = fb->get_width() / 2 - text_width / 2;
    int cursor_offset = 0;

    draw_dropping_number(fb, prevYear, year, animationPhase, left_offset);
    if(cursorPosition == YEAR) cursor_offset = left_offset;
    left_offset += 4 * font->width;
    
    fb->put_glyph(font, '/', left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(fb, prevMonth, month, animationPhase, left_offset);
    if(cursorPosition == MONTH) cursor_offset = left_offset;
    left_offset += 2 * font->width;

    fb->put_glyph(font, '/', left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(fb, prevDay, day, animationPhase, left_offset);
    if(cursorPosition == DAY) cursor_offset = left_offset;

    if(isShowingCursor) {
        fb->rect(cursor_offset - 2, 0, cursor_offset + (cursorPosition == YEAR ? 4 : 2) * font->width + 1, 15, false);
    }
}

void MenuDateSettingView::add_year(bool increment) {
    if(increment) {
        year += 1;
        if(year >= 2100) year = 1970;
    } else {
        year -= 1;
        if(year < 1970) year = 2099;
    }
    ensure_valid_day();
    if(animationPhase == -1) animationPhase = 0;
}

void MenuDateSettingView::add_month(bool increment) {
    if(increment) {
        month += 1;
        if(month > 12) {
            month = 1;
            add_year(true);
        }
    } else {
        month -= 1;
        if(month < 1) {
            month = 12;
            add_year(false);
        }
    }
    ensure_valid_day();
    if(animationPhase == -1) animationPhase = 0;
}

void MenuDateSettingView::add_day(bool increment) {
    if(increment) {
        day += 1;
        if(day > get_max_day_for_month()) {
            add_month(true);
            day = 1;
        }
    } else {
        day -= 1;
        if(day < 1) {
            add_month(false);
            day = get_max_day_for_month();
        }
    }
    if(animationPhase == -1) animationPhase = 0;
}

void MenuDateSettingView::ensure_valid_day() {
    day = std::min(day, get_max_day_for_month());
}

int MenuDateSettingView::get_max_day_for_month() {
    if (month == 2 && (year % 4 != 0 || (year % 100 == 0 && year % 400 != 0))) {
        return 28;
    } else if (month == 2) {
        return 29;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

void MenuDateSettingView::step() {
    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        onFinish(year, month, day);
    } 
    else if(hid_test_key_state_repetition(KEY_DOWN)) {
        switch(cursorPosition) {
            case YEAR: add_year(false); break;
            case MONTH: add_month(false); break;
            case DAY: add_day(false); break;
        }
    }
    else if(hid_test_key_state_repetition(KEY_UP)) {
        switch(cursorPosition) {
            case YEAR: add_year(true); break;
            case MONTH: add_month(true); break;
            case DAY: add_day(true); break;
        }
    }
    else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
        cursorPosition = (DateCursorPosition) (((int) cursorPosition) + 1);
        if(cursorPosition == 3) cursorPosition = YEAR;
    }

    if(animationPhase == 8) {
        beeper->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
    }
}