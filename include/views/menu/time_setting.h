#pragma once
#include <functional>
#include <views/framework.h>
#include <views/common/dropping_digits.h>
#include <fonts.h>
#include <sound/beeper.h>

class MenuTimeSettingView: public Renderable, DroppingDigits {
public:
    int hour;
    int minute;
    int second;

    MenuTimeSettingView(Beeper * b, int initialHour, int initialMinute, std::function<void(int hrs, int min, int sec)> onFinish, bool showSeconds = false, int initialSeconds = 0): 
        hour(initialHour),
        minute(initialMinute),
        second(initialSeconds),
        prevHour(initialHour),
        prevMinute(initialMinute),
        prevSecond(initialSeconds),
        showSeconds(showSeconds),
        animationPhase { -1 },
        cursorTimer { 0 },
        cursorPosition { CursorPosition::HOUR },
        isShowingCursor { false },
        onFinish(onFinish),
        beeper(b),
        DroppingDigits() {}

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
    bool showSeconds;
    int prevHour;
    int prevMinute;
    int prevSecond;
    int animationPhase;
    uint8_t cursorTimer;
    CursorPosition cursorPosition;
    bool isShowingCursor;
    void add_hr(bool);
    void add_min(bool);
    void add_sec(bool);
    Beeper * beeper;
};