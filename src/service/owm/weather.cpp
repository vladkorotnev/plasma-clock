#include <service/owm/weather.h>
#include <service/prefs.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static char LOG_TAG[] = "WEATHER";

static const char * currentApi = "http://api.openweathermap.org/data/3.0/onecall?lat=%s&lon=%s&APPID=%s";
static String apiKey;
static TickType_t interval;
static String latitude; 
static String longitude;
static TaskHandle_t hTask = NULL;
static SemaphoreHandle_t cacheSemaphore;
static current_weather_t cache = { 0 };
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

    static WiFiClient client;
    static HTTPClient http;

    static char url[256];
    snprintf(url, 150, currentApi, latitude.c_str(), longitude.c_str(), apiKey.c_str());

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
                        if(cache.description[0] >= 'a') {
                            cache.description[0] -= ('a' - 'A');
                        }
                    }
                    xSemaphoreGive(cacheSemaphore);
                    ESP_LOGI(LOG_TAG, "Weather refreshed");
                }
            }
        } else {
            ESP_LOGE(LOG_TAG, "Unexpected response code %i when refreshing", response);
            ESP_LOGE(LOG_TAG, "%s", http.getString());
            isFailure = true;
        }
        vTaskDelay(isFailure ? pdMS_TO_TICKS(5000) : interval);
    }
}


void weather_start() {
    if(hTask != NULL) {
        weather_stop();
    }

    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);

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