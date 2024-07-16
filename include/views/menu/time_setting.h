#pragma once
#include <functional>
#include <views/framework.h>
#include <fonts.h>
#include <sound/beeper.h>

class MenuTimeSettingView: public Renderable {
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
        font { &xnu_font },
        onFinish(onFinish),
        beeper(b) {}

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
    const font_definition_t * font;
    void draw_dropping_digit(FantaManipulator *, char, char, int, int);
    void draw_dropping_number(FantaManipulator *, int, int, int, int);
    void add_hr(bool);
    void add_min(bool);
    void add_sec(bool);
    Beeper * beeper;
};