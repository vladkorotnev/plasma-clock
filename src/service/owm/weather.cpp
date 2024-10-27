#include <service/owm/weather.h>
#include <service/prefs.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <os_config.h>
#include <service/localize.h>

static char LOG_TAG[] = "WEATHER";

static const char * currentApi = "http://api.openweathermap.org/data/3.0/onecall?lat=%s&lon=%s&APPID=%s&exclude=minutely,alerts&lang=%s";
static String apiKey;
static TickType_t interval;
static String latitude; 
static String longitude;
static TaskHandle_t hTask = NULL;
static SemaphoreHandle_t cacheSemaphore;
static SemaphoreHandle_t firstRunSemaphore;
static current_weather_t cache = { 0 };
static forecast_weather_t forecast_daily[FORECAST_WEATHER_DAYS] = { 0 };
static hourly_weather_t forecast_hourly[FORECAST_WEATHER_HOURS] = { 0 };
static volatile bool is_demoing = false;

weather_condition_t normalized_conditions(uint conditions) {
    if(conditions >= weather_condition_t::THUNDERSTORM && conditions < weather_condition_t::DRIZZLE) {
        return weather_condition_t::THUNDERSTORM;
    } else if(conditions >= weather_condition_t::DRIZZLE && conditions < weather_condition_t::RAIN) {
        return weather_condition_t::DRIZZLE;
    } else if(conditions >= weather_condition_t::RAIN && conditions < weather_condition_t::SNOW) {
        return weather_condition_t::RAIN;
    } else if(conditions >= weather_condition_t::SNOW && conditions < weather_condition_t::MIST) {
        return weather_condition_t::SNOW;
    } else if(conditions >= weather_condition_t::MIST && conditions < weather_condition_t::CLEAR) {
        return weather_condition_t::CLEAR;
    } else if(conditions >= weather_condition_t::CLEAR && conditions <= weather_condition_t::OVERCAST) {
        return (weather_condition_t) conditions;
    }

    ESP_LOGE(LOG_TAG, "Unknown conditions %u", conditions);

    return weather_condition_t::UNKNOWN;
}

extern "C" void WeatherTaskFunction( void * pvParameter );
void WeatherTaskFunction( void * pvParameter )
{
    ESP_LOGI(LOG_TAG, "Task started");

    EXT_RAM_ATTR static WiFiClient client;
    EXT_RAM_ATTR static HTTPClient http;

    EXT_RAM_ATTR static char url[256];

    static const char * langcode;
    switch(active_display_language()) {
        case DSPL_LANG_RU:
            langcode = "ru";
            break;
        case DSPL_LANG_EN:
        default:
            langcode = "en";
            break;
    }

    snprintf(url, 150, currentApi, latitude.c_str(), longitude.c_str(), apiKey.c_str(), langcode);

    bool isFailure = false;

    while(1) {
        isFailure = false;
        http.begin(client, url);
        ESP_LOGV(LOG_TAG, "Query: %s", url);
        int response = http.GET();
        if(response == 200) {
            JsonDocument response;

            DeserializationError error = deserializeJson(response, http.getStream());

            if (error) {
                ESP_LOGE(LOG_TAG, "Parse error: %s", error.c_str());
                isFailure = true;
            } else {
                if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
                    ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
                } else {
                    if(!is_demoing) {
                        cache.conditions = normalized_conditions(response["current"]["weather"][0]["id"]);
                        cache.temperature_kelvin = response["current"]["temp"];
                        cache.feels_like_kelvin = response["current"]["feels_like"];
                        cache.pressure_hpa = response["current"]["pressure"];
                        cache.humidity_percent = response["current"]["humidity"];
                        cache.windspeed_mps = response["current"]["wind_speed"];
                        cache.last_updated = xTaskGetTickCount();
                        String desc = response["current"]["weather"][0]["description"].as<String>();
                        strncpy(cache.description, desc.c_str(), sizeof(cache.description));
                        // Capitalize first letter
                        if(cache.description[0] >= 'a' && cache.description[0] <= 'z') {
                            cache.description[0] -= ('a' - 'A');
                        }
                    }

                    memset(&forecast_daily, 0, sizeof(forecast_daily));
                    if(response["daily"].is<JsonArray>()) {
                        int day_idx = -1;
                        for(auto day: response["daily"].as<JsonArray>()) {
                            if(day_idx >= 0) {
                                forecast_daily[day_idx] = {
                                    .day_temperature_kelvin = day["temp"]["day"],
                                    .night_temperature_kelvin = day["temp"]["night"],
                                    .pressure_hpa = day["pressure"],
                                    .conditions = normalized_conditions(day["weather"][0]["id"]),
                                    .precipitation_percentage = (unsigned int) trunc( day["pop"].as<float>() * 100.0 ),
                                    .date = unixtime_to_date(day["dt"].as<time_t>())
                                };
                            }
                            day_idx++;
                            if(day_idx == FORECAST_WEATHER_DAYS) break;
                        }
                    }

                    memset(&forecast_hourly, 0, sizeof(forecast_hourly));
                    if(response["hourly"].is<JsonArray>()) {
                        int hr_idx = 0;
                        for(auto hr: response["hourly"].as<JsonArray>()) {
                            forecast_hourly[hr_idx] = {
                                .temperature_kelvin = hr["temp"],
                                .pressure_hpa = hr["pressure"],
                                .conditions = normalized_conditions(hr["weather"][0]["id"]),
                                .precipitation_percentage = (unsigned int) trunc( hr["pop"].as<float>() * 100.0 ),
                                .time = unixtime_to_time(hr["dt"].as<time_t>())
                            };
                            hr_idx++;
                            if(hr_idx == FORECAST_WEATHER_HOURS) break;
                        }
                    }

                    xSemaphoreGive(cacheSemaphore);
                    ESP_LOGI(LOG_TAG, "Weather refreshed");
                }
            }
        } else {
            ESP_LOGE(LOG_TAG, "Unexpected response code %i when refreshing", response);
            isFailure = true;
        }
        xSemaphoreGive(firstRunSemaphore);
        vTaskDelay(isFailure ? pdMS_TO_TICKS(5000) : interval);
    }
}


void weather_start() {
    if(hTask != NULL) {
        weather_stop();
    }

    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);

    firstRunSemaphore = xSemaphoreCreateBinary();

    apiKey = prefs_get_string(PREFS_KEY_WEATHER_APIKEY, String(WEATHER_API_KEY));
    latitude = prefs_get_string(PREFS_KEY_WEATHER_LAT, String(WEATHER_LAT));
    longitude = prefs_get_string(PREFS_KEY_WEATHER_LON, String(WEATHER_LON));

    int interval_minutes = prefs_get_int(PREFS_KEY_WEATHER_INTERVAL_MINUTES);
    if(interval_minutes == 0) {
        interval_minutes = 60;
        prefs_set_int(PREFS_KEY_WEATHER_INTERVAL_MINUTES, interval_minutes);
    }
    interval = pdMS_TO_TICKS(interval_minutes * 60 * 1000);

    if(xTaskCreate(
        WeatherTaskFunction,
        "OWM",
        8192,
        nullptr,
        pisosTASK_PRIORITY_WEATHER,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    } else {
        // avoid core contention by waiting for the first request to go through before continuing with boot
        ESP_LOGI(LOG_TAG, "Waiting on first run semaphore");
        xSemaphoreTake(firstRunSemaphore, portMAX_DELAY);
    }
}

void weather_stop() {
    if(hTask != NULL) {
        vTaskDelete(hTask);
        hTask = NULL;
    }
}

bool weather_get_current(current_weather_t * dst) {
    if(dst == nullptr) return false;

    if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
        ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
        return false;
    }

    memcpy(dst, &cache, sizeof(current_weather_t));

    xSemaphoreGive(cacheSemaphore);
    return true;
}

const forecast_weather_t * weather_get_forecast(int day) {
    if(day < 0 || day >= FORECAST_WEATHER_DAYS) return nullptr;
    return &forecast_daily[day];
}

const hourly_weather_t * weather_get_hourly(int hour) {
    if(hour < 0 || hour >= FORECAST_WEATHER_HOURS) return nullptr;
    return &forecast_hourly[hour];
}

float convert_temperature(temperature_unit_t from, float inVal, temperature_unit_t unit) {
    if(from == unit) return inVal;
    
    float k = 0;

    switch(from) {
        case KELVIN: k = inVal; break;
        case CELSIUS: k = inVal + 273.15; break;
        case FAHRENHEIT: k = (inVal - 32.0) * 5 / 9 + 273.15; break;
    }

    switch(unit) {
        case KELVIN: return k;
        case CELSIUS: return k - 273.15;
        case FAHRENHEIT: return (k - 273.15) * 9 / 5 + 32;
    }

    return 0.0; // unreachable
}

void weather_set_demo(current_weather_t * demo) {
    if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
        ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
        return;
    }

    is_demoing = true;
    memcpy(&cache, demo, sizeof(current_weather_t));
    cache.conditions = normalized_conditions(cache.conditions);

    xSemaphoreGive(cacheSemaphore);
}

temperature_unit_t preferred_temperature_unit() {
    return prefs_get_bool(PREFS_KEY_WEATHER_USE_FAHRENHEIT) ? FAHRENHEIT : CELSIUS;
}
