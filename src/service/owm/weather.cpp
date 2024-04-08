#include <service/owm/weather.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static char LOG_TAG[] = "WEATHER";

static const char * currentApi = "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&APPID=%s";
static const char * apiKey;
static TickType_t interval;
static const char * latitude; 
static const char * longitude;
static TaskHandle_t hTask = NULL;
static SemaphoreHandle_t cacheSemaphore;
static current_weather_t cache = { 0 };

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

    return weather_condition_t::UNKNOWN;
}

extern "C" void WeatherTaskFunction( void * pvParameter );
void WeatherTaskFunction( void * pvParameter )
{
    ESP_LOGI(LOG_TAG, "Task started");

    static WiFiClient client;
    static HTTPClient http;

    static char url[150];
    snprintf(url, 150, currentApi, latitude, longitude, apiKey);

    while(1) {
        // http.begin(client, url);
        // ESP_LOGV(LOG_TAG, "Query: %s", url);
        // int response = http.GET();
        // if(response == 200) {
        //     JsonDocument response;

        //     DeserializationError error = deserializeJson(response, http.getStream());

        //     if (error) {
        //         ESP_LOGE(LOG_TAG, "Parse error: %s", error.c_str());
        //     } else {
        //         if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
        //             ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
        //         } else {
        //             cache.conditions = normalized_conditions(response["weather"]["id"]);
        //             cache.temperature_kelvin = response["main"]["temp"];
        //             cache.feels_like_kelvin = response["main"]["feels_like"];
        //             cache.pressure_hpa = response["main"]["pressure"];
        //             cache.humidity_percent = response["main"]["humidity"];
        //             xSemaphoreGive(cacheSemaphore);
        //             ESP_LOGI(LOG_TAG, "Weather refreshed");
        //         }
        //     }
        // } else {
        //     ESP_LOGE(LOG_TAG, "Unexpected response code %i when refreshing", response);
        //     ESP_LOGE(LOG_TAG, "%s", http.getString());
        // }
        vTaskDelay(interval);
    }
}


void weather_start(const char* api_key, TickType_t update_interval, const char * lat, const char * lon) {
    if(hTask != NULL) {
        weather_stop();
    }

    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);

    apiKey = api_key;
    latitude = lat;
    longitude = lon;
    interval = update_interval;

    if(xTaskCreate(
        WeatherTaskFunction,
        "OWM",
        4096,
        nullptr,
        8,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
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

float kelvin_to(float k, temperature_unit_t unit) {
    switch(unit) {
        case KELVIN: return k;
        case CELSIUS: return k - 273.15;
        case FAHRENHEIT: return kelvin_to(k, CELSIUS) * 9 / 5 + 32;
    }

    return k;
}