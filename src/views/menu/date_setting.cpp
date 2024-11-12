#include <views/menu/date_setting.h>
#include <input/keys.h>

void MenuDateSettingView::prepare() {
    yearView->value = year;
    monthView->value = month;
    dayView->value = day;
    Composite::prepare();
}

void MenuDateSettingView::render(FantaManipulator *fb) {
    cursorTimer++;
    if(cursorTimer == 30) {
        cursorTimer = 0;
        isShowingCursor = !isShowingCursor;
    }

    Composite::render(fb);
    
    fb->put_glyph(font, '/', yearView->x_offset + yearView->width, 0);
    fb->put_glyph(font, '/', monthView->x_offset + monthView->width, 0);

    DroppingDigitView * editedView;
    switch(cursorPosition) {
        case DAY: editedView = dayView; break;
        case MONTH: editedView = monthView; break;
        case YEAR: editedView = yearView; break;
    }
    if(isShowingCursor) {
        fb->rect(editedView->x_offset - 2, 0, editedView->x_offset + editedView->width + 1, 15, false);
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
    yearView->value = year;
    ensure_valid_day();
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
    monthView->value = month;
    ensure_valid_day();
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
    dayView->value = day;
}

void MenuDateSettingView::ensure_valid_day() {
    day = std::min(day, get_max_day_for_month());
    dayView->value = day;
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

    Composite::step();
}