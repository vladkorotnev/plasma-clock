#pragma once
#include <freertos/FreeRTOS.h>
#include <service/time.h>

#define FORECAST_WEATHER_DAYS 4
#define FORECAST_WEATHER_HOURS 48

typedef enum weather_condition_code {
    UNKNOWN_CONDITIONS = 0,

    THUNDERSTORM = 200,

    DRIZZLE = 300,

    RAIN = 500,

    SNOW = 600,

    MIST = 701, 

    CLEAR = 800,
    CLOUDS = 801,
    SCATTERED_CLOUDS = 802,
    BROKEN_CLOUDS = 803,
    OVERCAST = 804
} weather_condition_t;

typedef struct current_weather {
    float temperature_kelvin;
    float feels_like_kelvin;
    float windspeed_mps;
    int pressure_hpa;
    int humidity_percent;
    weather_condition_t conditions;
    TickType_t last_updated;
    char description[64];
} current_weather_t;

typedef struct forecast_weather {
    float day_temperature_kelvin;
    float night_temperature_kelvin;
    int pressure_hpa;
    weather_condition_t conditions;
    unsigned int precipitation_percentage;
    tk_date_t date;
} forecast_weather_t;

typedef struct hourly_weather {
    float temperature_kelvin;
    int pressure_hpa;
    weather_condition_t conditions;
    unsigned int precipitation_percentage;
    tk_time_of_day_t time;
} hourly_weather_t;

typedef enum temperature_unit {
    KELVIN = 'K',
    CELSIUS = 'C',
    FAHRENHEIT = 'F'
} temperature_unit_t;

/// @brief Start up the periodic weather update service
void weather_start();
/// @brief Stop the weather update service
void weather_stop();
/// @brief Get the current weather
/// @return Whether the passed structure was updated correctly
bool weather_get_current(current_weather_t *);
/// @brief Set a dummy weather state for demo purposes
void weather_set_demo(current_weather_t *);

const forecast_weather_t * weather_get_forecast(int day_from_now);
const hourly_weather_t * weather_get_hourly(int hour_from_now);

temperature_unit_t preferred_temperature_unit();

float convert_temperature(temperature_unit_t from, float, temperature_unit_t to = preferred_temperature_unit());
