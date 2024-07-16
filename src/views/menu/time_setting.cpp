#include <views/menu/time_setting.h>
#include <input/keys.h>

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
            prevSecond = second;
        }
    } else {
        prevHour = hour;
        prevMinute = minute;
        prevSecond = second;
    }

    cursorTimer++;
    if(cursorTimer == 30) {
        cursorTimer = 0;
        isShowingCursor = !isShowingCursor;
    }

    int char_count = showSeconds ? 8 : 5; // XX:XX:XX or XX:XX
    int text_width = char_count * font->width;
    int left_offset = fb->get_width() / 2 - text_width / 2;
    int cursor_offset = 0;

    draw_dropping_number(fb, prevHour, hour, animationPhase, left_offset);
    if(cursorPosition == CursorPosition::HOUR) cursor_offset = left_offset;
    left_offset += 2 * font->width;
    
    fb->put_glyph(font, ':', left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(fb, prevMinute, minute, animationPhase, left_offset);
    if(cursorPosition == CursorPosition::MINUTE) cursor_offset = left_offset;
    left_offset += 2 * font->width;

    if (showSeconds) {
        fb->put_glyph(font, ':', left_offset, 0);
        left_offset += font->width;

        draw_dropping_number(fb, prevSecond, second, animationPhase, left_offset);
        if(cursorPosition == CursorPosition::SECOND) cursor_offset = left_offset;
    }

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
    if(animationPhase == -1) animationPhase = 0;
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

    if(animationPhase == 8) {
        beeper->beep_blocking(CHANNEL_AMBIANCE, 100, 10);
    }
}
