#pragma once
#include <functional>
#include <views/framework.h>
#include <views/common/dropping_digits.h>
#include <fonts.h>
#include <sound/beeper.h>

class MenuTimeSettingView: public Composite {
public:
    int hour;
    int minute;
    int second;

    MenuTimeSettingView(Beeper * b, int initialHour, int initialMinute, std::function<void(int hrs, int min, int sec)> onFinish, bool showSeconds = false, int initialSeconds = 0): 
        hour(initialHour),
        minute(initialMinute),
        second(initialSeconds),
        hourView(new DroppingDigitView(2, initialHour, b)),
        minuteView(new DroppingDigitView(2, initialMinute, b)),
        secondView(new DroppingDigitView(2, initialSeconds, b)),
        showSeconds(showSeconds),
        cursorTimer { 0 },
        cursorPosition { CursorPosition::HOUR },
        isShowingCursor { false },
        onFinish(onFinish),
        beeper(b) {
            int char_count = showSeconds ? 8 : 5; // XX:XX:XX or XX:XX
            int text_width = char_count * xnu_font.width;
            int left_offset = HWCONF_DISPLAY_WIDTH_PX/2 - text_width/2;

            hourView->x_offset = left_offset;
            minuteView->x_offset = hourView->x_offset + hourView->width + xnu_font.width;
            secondView->x_offset = minuteView->x_offset + minuteView->width + xnu_font.width;

            add_composable(hourView);
            add_composable(minuteView);
            if(showSeconds) {
                add_composable(secondView);
            }
        }

    void prepare();
    void step();
    void render(FantaManipulator *fb);

protected:
    std::function<void(int h, int m, int s)> onFinish;
private:
    enum CursorPosition {
        HOUR,
        MINUTE,
        SECOND
    };
    DroppingDigitView * hourView;
    DroppingDigitView * minuteView;
    DroppingDigitView * secondView;
    bool showSeconds;
    uint8_t cursorTimer;
    CursorPosition cursorPosition;
    bool isShowingCursor;
    void add_hr(bool);
    void add_min(bool);
    void add_sec(bool);
    Beeper * beeper;
};