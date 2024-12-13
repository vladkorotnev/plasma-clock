#pragma once
#include <views/framework.h>
#include <vector>

class DailyForecastView: public Screen {
public:
    bool controllable;
    DailyForecastView(bool controllable = false);
    void prepare() override;
    void render(FantaManipulator*) override;
    void step() override;

private:
    class Column;
    std::vector<Column *> columns;
    int offset;
    int total_per_screen;
    int one_widget_width;
    int screen_width;
    bool need_width_update;
    void update_width();
    void update_data();
};