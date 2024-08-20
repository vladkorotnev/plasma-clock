#pragma once
#include "chart_common.h"
#include <service/owm/weather.h>
#include <service/time.h>

class WeatherPressureChart: public WeatherChartCommon {
public:
    WeatherPressureChart(): WeatherChartCommon() {
        filled = false;
        show_maximum = true;
        show_minimum = true;
        autoscale = true;
        hint = "Pressure hPa";
    }

    void prepare() override {
        points.clear();
        cursor_index = -1;
        tk_time_of_day_t now = get_current_time_coarse();
        int p = 0;
        for(int i = 0; i < 36; i++) {
            const hourly_weather_t * f = weather_get_hourly(i);
            if(f != nullptr) {
                for(int x = 0; x < 3; x++) {
                    if(cursor_index == -1 && f->time.hour == now.hour) {
                        if(now.minute >= 33 * x && now.minute < 33 * (x + 1)) {
                            cursor_index = p;
                        }
                    }
                    points.push_back({
                        .annotation = (f->time.hour % 6 == 0 && x == 0 && i > 3) ? f->time.hour : -1,
                        .value = f->pressure_hpa
                    });
                    p++;
                }
            }
        }
        WeatherChartCommon::prepare();
        minimum -= 5;
        maximum += 5;
    }

    int desired_display_time() override { return DISP_TIME_NO_OVERRIDE; }
};