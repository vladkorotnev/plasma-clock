#pragma once
#include <views/framework.h>
#include <views/weather/current_weather.h>
#include <views/weather/daily_forecast.h>
#include <views/weather/chart_precipitation.h>
#include <views/weather/chart_pressure.h>
#include <state.h>

class AppShimWeather: public ListView {
public:
    AppShimWeather() {
        conditions = new CurrentWeatherView();
        forecast = new DailyForecastView(true);
        precipitation = new WeatherPrecipitationChart();
        pressure = new WeatherPressureChart();
        
        add_view(conditions);
        add_view(precipitation);
        add_view(pressure);
        add_view(forecast);
    }

    void step() override {
        ListView::step();
        if(hid_test_key_state(KEY_LEFT) == KEYSTATE_HIT) {
            pop_state(STATE_WEATHER, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        }
    }

private:
    WeatherPrecipitationChart * precipitation;
    WeatherPressureChart * pressure;
    CurrentWeatherView * conditions;
    DailyForecastView * forecast;
};