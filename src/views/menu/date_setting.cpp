#include <views/menu/date_setting.h>
#include <input/keys.h>

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