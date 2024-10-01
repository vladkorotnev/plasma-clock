#pragma once
#include <graphics/framebuffer.h>
#include <sound/beeper.h>
#include <service/time.h>
#include <views/common/view.h>
#include <views/common/dropping_digits.h>

/// @brief A simple clock view
class SimpleClock: public Screen, DroppingDigits {
public:
    SimpleClock();
    void render_plane(FantaManipulator*, RenderPlane);
    void step();
private:
    class AmPmLabel;
    tk_time_of_day_t now;
    tk_time_of_day_t next_time;
    int phase;
    char separator;
    bool blink_separator;
    bool show_seconds;
    AmPmLabel * pm_label;
};