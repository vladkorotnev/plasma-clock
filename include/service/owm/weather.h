#pragma once
#include <freertos/FreeRTOS.h>

typedef enum weather_condition_code {
    UNKNOWN = 0,

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

typedef enum temperature_unit {
    KELVIN,
    CELSIUS,
    FAHRENHEIT
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

float kelvin_to(float, temperature_unit_t);