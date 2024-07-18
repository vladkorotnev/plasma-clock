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
    .mask = nullptr
};

inline void itoa_padded(uint i, char * a) {
    a[0] = '0' + (i / 10);
    a[1] = '0' + (i % 10);
    a[2] = 0;
}

NextAlarmView::NextAlarmView(): DroppingDigits() {
    show_alarm = false;
    time_remaining = {0};
    font = &xnu_font;
    phase = -1;
    disp_h = 0;
    disp_m = 0;
    next_h = 0;
    next_m = 0;
}

void NextAlarmView::prepare() {
    recalculate();
    disp_h = next_h;
    disp_m = next_m;
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
    } else {
        show_alarm = false;
    }
}

void NextAlarmView::step() {
    recalculate();
    if(time_remaining.hour > 0) {
        next_h = time_remaining.hour;
        next_m = time_remaining.minute;
    } else {
        next_h = time_remaining.minute;
        next_m = time_remaining.second;
    }

    // this pattern actually makes sense, extract it into DroppingDigits?
    if((next_h != disp_h || next_m != disp_m) && phase == -1) {
        phase = 0;
    } else if (phase < 17 && phase > -1) {
        phase++;
    }

    if(phase == 17) {
        disp_h = next_h;
        disp_m = next_m;
        phase = -1;
    }
}

void NextAlarmView::render(FantaManipulator *fb) {
    fb->put_sprite(&sleep_icns, 0, 0);

    int char_count = 5; // XX:XX
    int text_width = char_count * font->width;
    int left_offset = (fb->get_width() - alarm_icns.width) / 2 - text_width / 2 + alarm_icns.width;

    draw_dropping_number(fb, disp_h, next_h, phase == -1 ? 0 : phase, left_offset);
    left_offset += 2 * font->width;
    
    fb->put_glyph(font, ':', left_offset, 0);
    left_offset += font->width;

    draw_dropping_number(fb, disp_m, next_m, phase == -1 ? 0 : phase, left_offset);
}

int NextAlarmView::desired_display_time() {
    recalculate();
    if(show_alarm && time_remaining.hour < 8) {
        return DISP_TIME_NO_OVERRIDE;
    } else {
        return DISP_TIME_DONT_SHOW;
    }
}
