#pragma once
#include <views/framework.h>

class DailyForecastView: public Screen {
public:
    bool controllable;
    DailyForecastView(bool controllable = false);
    ~DailyForecastView();
    void prepare() override;
    void step() override;

private:
    class Column;
    Column * leftView;
    Column * rightView;
    int offset;
};