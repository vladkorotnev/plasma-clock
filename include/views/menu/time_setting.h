#pragma once
#include <functional>
#include <views/framework.h>
#include <fonts.h>
#include <sound/beeper.h>

class MenuTimeSettingView: public Renderable {
public:
    int hour;
    int minute;

    MenuTimeSettingView(Beeper * b, int initialHour, int initialMinute, std::function<void(int hrs, int min)> onFinish): 
        hour(initialHour),
        minute(initialMinute),
        prevHour(initialHour),
        prevMinute(initialMinute),
        animationPhase { -1 },
        cursorTimer { 0 },
        isEditingMinutes { false },
        isShowingCursor { false },
        font { &xnu_font },
        onFinish(onFinish),
        beeper(b) {}

    void step();
    void render(FantaManipulator *fb);

protected:
    std::function<void(int h, int m)> onFinish;
private:
    int prevHour;
    int prevMinute;
    int animationPhase;
    uint8_t cursorTimer;
    bool isEditingMinutes;
    bool isShowingCursor;
    const font_definition_t * font;
    void draw_dropping_digit(FantaManipulator *, char, char, int, int);
    void draw_dropping_number(FantaManipulator *, int, int, int, int);
    void add_hr(bool);
    void add_min(bool);
    Beeper * beeper;
};