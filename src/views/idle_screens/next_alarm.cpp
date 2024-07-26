#include <views/idle_screens/next_alarm.h>
#include <rsrc/common_icons.h>
#include <fonts.h>
#include <esp32-hal-log.h>

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

    hourView = new DroppingDigitView(2, 0);
    minuteView = new DroppingDigitView(2, 0);

    int char_count = 5; // XX:XX
    int text_width = char_count * xnu_font.width;
    int left_offset = (HWCONF_DISPLAY_WIDTH_PX - alarm_icns.width) / 2 - text_width / 2 + alarm_icns.width;
    hourView->x_offset = left_offset;
    minuteView->x_offset = hourView->x_offset + hourView->width + xnu_font.width;

    add_composable(hourView);
    add_composable(minuteView);
}

NextAlarmView::~NextAlarmView() {
    delete hourView, minuteView;
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
    Composite::step();
}

void NextAlarmView::render(FantaManipulator *fb) {
    Composite::render(fb);
    fb->put_sprite(&sleep_icns, 0, 0);
    fb->put_glyph(&xnu_font, ':', hourView->x_offset + hourView->width, 0);
}

int NextAlarmView::desired_display_time() {
    recalculate();
    if(show_alarm && time_remaining.hour < 8) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}
