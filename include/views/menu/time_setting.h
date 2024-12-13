#pragma once
#include <functional>
#include <views/framework.h>
#include <views/common/dropping_digits.h>
#include <graphics/font.h>
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
        font(find_font(FONT_STYLE_CLOCK_FACE)),
        showSeconds(showSeconds),
        cursorTimer { 0 },
        cursorPosition { CursorPosition::HOUR },
        isShowingCursor { false },
        onFinish(onFinish),
        beeper(b) {
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
    const font_definition_t * font;
    bool showSeconds;
    uint8_t cursorTimer;
    CursorPosition cursorPosition;
    bool isShowingCursor;
    void add_hr(bool);
    void add_min(bool);
    void add_sec(bool);
    Beeper * beeper;
};