#pragma once
#include "chart_common.h"
#include <service/owm/weather.h>
#include <service/time.h>

class WeatherPrecipitationChart: public WeatherChartCommon {
public:
    WeatherPrecipitationChart(): WeatherChartCommon() {
        filled = true;
        show_maximum = false;
        show_minimum = false;
        autoscale = false;
        minimum = 0;
        maximum = 100;
        hint = "PoP, %";
    }

    void prepare() override {
        points.clear();
        cursor_index = -1;
        tk_time_of_day_t now = get_current_time_coarse();
        int p = 0;
        for(int i = 0; i < FORECAST_WEATHER_HOURS; i++) {
            const hourly_weather_t * f = weather_get_hourly(i);
            if(f != nullptr) {
                for(int x = 0; x < 4; x++) {
                    if(cursor_index == -1 && f->time.hour == now.hour) {
                        if(now.minute >= 15 * x && now.minute < 15 * (x + 1)) {
                            cursor_index = p;
                        }
                    }
                    points.push_back({
                        .annotation = (f->time.hour % 6 == 0 && x == 0 && i > 3) ? f->time.hour : -1,
                        .value = interpolate(f->precipitation_percentage)
                    });
                    p++;
                }
            }
        }
        WeatherChartCommon::prepare();
    }

private:
    int interpolate(unsigned int pct) {
        if(pct < 7) return 7;
        return pct;  
    }
};