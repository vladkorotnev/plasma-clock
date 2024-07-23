#pragma once
#include <views/framework.h>
#include <service/time.h>
#include <service/alarm.h>
#include <views/common/dropping_digits.h>

/// @brief A view that shows the time remaining until the next alarm.
class NextAlarmView: public Screen, DroppingDigits {
public:
    NextAlarmView();
    void prepare();
    void render(FantaManipulator*);
    void step();
    int desired_display_time();

private:
    void recalculate();
    tk_time_of_day_t time_remaining;
    bool show_alarm;

    // For dropping digits
    int disp_h, next_h, disp_m, next_m;
    int phase;
    const font_definition_t * font;
};

