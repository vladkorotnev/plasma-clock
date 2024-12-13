#include <views/framework.h>
#include <views/common/dropping_digits.h>
#include <sound/beeper.h>
#include <graphics/font.h>
#include <functional>

class MenuDateSettingView: public Composite {
public:
    int year;
    int month;
    int day;

    MenuDateSettingView(Beeper * b, int initialYear, int initialMonth, int initialDay, std::function<void(int yrs, int mth, int dy)> onFinish): 
        year(initialYear),
        month(initialMonth),
        day(initialDay),
        font(find_font(FONT_STYLE_CLOCK_FACE)),
        yearView(new DroppingDigitView(4, initialYear, b)),
        monthView(new DroppingDigitView(2, initialMonth, b)),
        dayView(new DroppingDigitView(2, initialDay, b)),
        cursorTimer { 0 },
        cursorPosition { YEAR },
        isShowingCursor { false },
        onFinish(onFinish),
        beeper(b) {
            add_composable(yearView);
            add_composable(monthView);
            add_composable(dayView);
        }

    ~MenuDateSettingView() {
        delete yearView, monthView, dayView;
    }

    void prepare();
    void step();
    void render(FantaManipulator *fb);

protected:
    std::function<void(int yrs, int mth, int dy)> onFinish;
private:
    enum DateCursorPosition {
        YEAR,
        MONTH,
        DAY
    };
    DroppingDigitView * yearView;
    DroppingDigitView * monthView;
    DroppingDigitView * dayView;
    uint8_t cursorTimer;
    DateCursorPosition cursorPosition;
    bool isShowingCursor;
    void add_year(bool);
    void add_month(bool);
    void add_day(bool);
    void ensure_valid_day();
    int get_max_day_for_month();
    Beeper * beeper;
    const font_definition_t * font;
};