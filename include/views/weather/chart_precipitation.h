#pragma once
#include "chart_common.h"
#include <service/owm/weather.h>
#include <service/time.h>
#include <service/localize.h>

class WeatherPrecipitationChart: public WeatherChartCommon {
public:
    WeatherPrecipitationChart(bool hint_enabled = false): WeatherChartCommon() {
        is_on_screen = false;
        filled = true;
        show_maximum = false;
        show_minimum = false;
        autoscale = false;
        minimum = 0;
        maximum = 95;
        hint = hint_enabled ? localized_string("PoP, %") : nullptr;
    }

    void prepare() override {
        load_data();
        WeatherChartCommon::prepare();
        is_on_screen = true;
    }

    int desired_display_time() override {
        if(!is_on_screen)
            load_data();
        
        for(auto p: points) {
            if(p.value > 7) return DISP_TIME_NO_OVERRIDE;
        }
        return DISP_TIME_DONT_SHOW; // don't show if no data
    }

    void cleanup() override {
        is_on_screen = false;
        WeatherChartCommon::cleanup();
    }

private:
    bool is_on_screen = false;

    void load_data() {
        points.clear();
        cursor_index = -1;
        tk_time_of_day_t now = get_current_time_coarse();
        int p = 0;
        for(int i = 0; i < 24; i++) {
            const hourly_weather_t * f = weather_get_hourly(i);
            if(f != nullptr) {
                for(int x = 0; x < 4; x++) {
                    if(cursor_index == -1 && f->time.hour == now.hour) {
                        if(now.minute >= 15 * x && now.minute < 15 * (x + 1)) {
                            cursor_index = p;
                        }
                    }
                    int val = f->precipitation_percentage;
                    if(x == 0) {
                        const hourly_weather_t * prev = weather_get_hourly(i - 1);
                        if(prev != nullptr) {
                            val += prev->precipitation_percentage;
                            val /= 2;
                        }
                    } else if(x == 3) {
                        const hourly_weather_t * next = weather_get_hourly(i + 1);
                        if(next != nullptr) {
                            val += next->precipitation_percentage;
                            val /= 2;
                        }
                    }
                    points.push_back({
                        .annotation = (f->time.hour % 6 == 0 && x == 0 && i > 3) ? f->time.hour : -1,
                        .value = interpolate(val)
                    });
                    p++;
                }
            }
        }
    }

    int interpolate(unsigned int pct) {
        if(pct < 7) return 7;
        return pct;  
    }
};