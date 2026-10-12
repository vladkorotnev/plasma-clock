#include <views/idle_screens/next_alarm.h>
#include <rsrc/common_icons.h>
#include <graphics/font.h>
#include <esp32-hal-log.h>
#include <service/localize.h>
#include <input/keys.h>

static const uint8_t sleep_icns_data[] = {
    // By PiiXL
    0x00, 0x00, 0x00, 0x1e, 0x1f, 0xc4, 0x3f, 0xc8, 0x7f, 0xde, 0xff, 0xe0, 0xff, 0xfc, 0xdb, 0x6c, 
    0xe7, 0x9c, 0xff, 0xfc, 0xff, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0x7f, 0xf8, 0x3f, 0xf0, 0x1f, 0xe0,
};

static const sprite_t sleep_icns = {
    .width = 16, .height = 16,
    .data = sleep_icns_data,
    .mask = nullptr,
    .format = SPRFMT_HORIZONTAL
};


NextAlarmView::NextAlarmView() {
    show_alarm = false;
    time_remaining = {0};
    disp_h = 0;
    disp_m = 0;
    dismiss_button_was_pressed = false;
    dismiss_hold_remain = 0;
    dismiss_completed = false;
    framecount = 0;

    arrows = new TouchArrowOverlay();
    arrows->active = true;
    arrows->left = false;
    arrows->top = false;
    arrows->right = true;
    arrows->bottom = false;

    hourView = new DroppingDigitView(2, 0);
    minuteView = new DroppingDigitView(2, 0);
    font = find_font(FONT_STYLE_CLOCK_FACE);

    add_composable(hourView);
    add_composable(minuteView);
}

NextAlarmView::~NextAlarmView() {
    delete hourView;
    delete minuteView;
    delete arrows;
}

void NextAlarmView::prepare() {
    recalculate();
    hourView->value = disp_h;
    minuteView->value = disp_m;
    Composite::prepare();
}

void NextAlarmView::recalculate() {
    const alarm_setting_t * alarm = get_upcoming_alarm();
    if(alarm != nullptr) {
        tk_time_of_day_t now = get_current_time_precise();
        tk_time_of_day_t alarm_time = { 0 };
        alarm_time.hour = alarm->hour;
        alarm_time.minute = alarm->minute;
        
        time_remaining = alarm_time - now;

        show_alarm = time_remaining.hour < 8;

        if(time_remaining.hour > 0) {
            disp_h = time_remaining.hour;
            disp_m = time_remaining.minute;
        } else {
            disp_h = time_remaining.minute;
            disp_m = time_remaining.second;
        }
    } else {
        show_alarm = false;
    }
}

void NextAlarmView::step() {
    recalculate();
    hourView->value = disp_h;
    minuteView->value = disp_m;
    bool dismiss_is_pressed = hid_test_key_state(KEY_RIGHT);

    if (dismiss_button_was_pressed) {
        if (dismiss_is_pressed) {
            if (dismiss_hold_remain <= 0 && !dismiss_completed) {
                dismiss_upcoming_alarm();
                dismiss_completed = true;
            }
        } else {
            dismiss_button_was_pressed = false;
            dismiss_completed = false;
        }
    } else {
        dismiss_button_was_pressed = dismiss_is_pressed;
        dismiss_completed = false;
    }

    Composite::step();
}

void NextAlarmView::render(FantaManipulator *fb) {
    framecount++;

    if (!dismiss_button_was_pressed) {
        // Normal view state
        dismiss_hold_remain = fb->get_width();
        int char_count = 5; // XX:XX
        int text_width = char_count * font->width;
        int left_offset = (fb->get_width() - alarm_icns.width) / 2 - text_width / 2 + alarm_icns.width;
        hourView->x_offset = left_offset;
        minuteView->x_offset = hourView->x_offset + hourView->width + font->width;

        Composite::render(fb);
        fb->put_sprite(&sleep_icns, 0, 0);
        fb->put_glyph(font, ':', hourView->x_offset + hourView->width, 0);
    } else {
        // Dismiss hold state
        static const font_definition_t * tall_font = find_font(FONT_STYLE_TALL_TEXT);
        static const char * hold_str = localized_string("DISMISS");
        fb->put_string(tall_font, hold_str, fb->get_width() - 12 - measure_string_width(tall_font, hold_str), 0);
        arrows->render(fb);
        if(dismiss_hold_remain > 0 && framecount % 2 == 0) dismiss_hold_remain -= 1;
        FantaManipulator *holdProgress = fb->slice(0,  fb->get_width() - dismiss_hold_remain);
        holdProgress->invert();
        delete holdProgress;
    }
}

int NextAlarmView::desired_display_time() {
    recalculate();
    if(dismiss_button_was_pressed) {
        return DISP_TIME_ATTENTION;
    }
    else if(show_alarm && time_remaining.hour < 8) {
        return DISP_TIME_NO_OVERRIDE;
    } 
    else {
        return DISP_TIME_DONT_SHOW;
    }
}
