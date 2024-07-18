#pragma once
#include <views/framework.h>
#include <service/time.h>
#include <service/alarm.h>

/// @brief A view that shows the time remaining until the next alarm.
class NextAlarmView: public Screen {
public:
    NextAlarmView();
    void prepare();
    void render(FantaManipulator*);
    int desired_display_time();

private:
    tk_time_of_day_t time_remaining;
    bool show_alarm;
    const font_definition_t * font;
};

