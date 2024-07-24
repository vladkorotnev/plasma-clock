#include <views/menu/time_setting.h>
#include <input/keys.h>

void MenuTimeSettingView::prepare() {
    hourView->value = hour;
    minuteView->value = minute;
    secondView->value = second;
    Composite::prepare();
}

void MenuTimeSettingView::render(FantaManipulator *fb) {
    cursorTimer++;
    if(cursorTimer == 30) {
        cursorTimer = 0;
        isShowingCursor = !isShowingCursor;
    }

    Composite::render(fb);

    fb->put_glyph(&xnu_font, ':', hourView->x_offset + hourView->width, 0);

    if (showSeconds) {
        fb->put_glyph(&xnu_font, ':', minuteView->x_offset + minuteView->width, 0);
    }

    DroppingDigitView * editedView;
    switch(cursorPosition) {
        case HOUR: editedView = hourView; break;
        case MINUTE: editedView = minuteView; break;
        case SECOND: editedView = secondView; break;
    }

    if(isShowingCursor) {
        fb->rect(editedView->x_offset - 2, 0, editedView->x_offset + editedView->width + 1, 15, false);
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

    hourView->value = hour;
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
    minuteView->value = minute;
}

void MenuTimeSettingView::add_sec(bool increment) {
    if(increment) {
        second += 1;
        if(second > 59) {
            second = 0;
            add_min(true);
        }
    } else {
        second -= 1;
        if(second < 0) {
            second = 59;
            add_min(false);
        }
    }
    secondView->value = second;
}

void MenuTimeSettingView::step() {
    if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
        onFinish(hour, minute, second);
    } 
    else if(hid_test_key_state_repetition(KEY_DOWN)) {
        switch (cursorPosition) {
            case CursorPosition::HOUR: add_hr(false); break;
            case CursorPosition::MINUTE: add_min(false); break;
            case CursorPosition::SECOND: add_sec(false); break;
        }
    }
    else if(hid_test_key_state_repetition(KEY_UP)) {
        switch (cursorPosition) {
            case CursorPosition::HOUR: add_hr(true); break;
            case CursorPosition::MINUTE: add_min(true); break;
            case CursorPosition::SECOND: add_sec(true); break;
        }
    }
    else if(hid_test_key_state(KEY_RIGHT) == KEYSTATE_HIT) {
        if (showSeconds) {
            cursorPosition = (CursorPosition) (((int) cursorPosition) + 1);
            if(cursorPosition == 3) cursorPosition = CursorPosition::HOUR;
        } else {
            cursorPosition = (cursorPosition == CursorPosition::HOUR) ? CursorPosition::MINUTE : CursorPosition::HOUR;
        }
    }

    Composite::step();
}
