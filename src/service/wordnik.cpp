#include <service/wordnik.h>
#include <service/prefs.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static char LOG_TAG[] = "WORDNIK";

static const char * currentApi = "https://api.wordnik.com/v4/words.json/wordOfTheDay?api_key=%s";
static String apiKey;
static TickType_t interval;

static TaskHandle_t hTask = NULL;

static SemaphoreHandle_t cacheSemaphore;
static char wordCache[128];
static char definitionCache[256];
static TickType_t lastUpdate = 0;

TickType_t wotd_get_last_update() {
    return lastUpdate;
}

bool wotd_get_current(char * w, size_t wsz, char * d, size_t dsz) {
    if(w == nullptr || d == nullptr) return false;

    if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
        ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
        return false;
    }

    strncpy(w, wordCache, wsz-1);
    strncpy(d, definitionCache, dsz-1);

    xSemaphoreGive(cacheSemaphore);
    return true;
}

void WordnikTaskFunction( void * pvParameter )
{
    ESP_LOGI(LOG_TAG, "Task started");

    static WiFiClientSecure client;
    static HTTPClient http;

    static char url[128];
    snprintf(url, 150, currentApi, apiKey.c_str());

    while(1) {
        http.begin(client, url);
        client.setInsecure();
        ESP_LOGV(LOG_TAG, "Query: %s", url);
        int response = http.GET();
        if(response == 200) {
            JsonDocument response;

            DeserializationError error = deserializeJson(response, http.getStream());

            if (error) {
                ESP_LOGE(LOG_TAG, "Parse error: %s", error.c_str());
            } else {
                if(!xSemaphoreTake(cacheSemaphore, portMAX_DELAY)) {
                    ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
                } else {
                    String w = response["word"].as<String>();
                    String d = response["definitions"][0]["text"].as<String>();

                    strncpy(wordCache, w.c_str(), sizeof(wordCache));
                    if(wordCache[0] >= 'a') {
                        wordCache[0] -= ('a' - 'A');
                    }
                    strncpy(definitionCache, d.c_str(), sizeof(definitionCache));
                    definitionCache[sizeof(definitionCache) - 1] = 0;

                    lastUpdate = xTaskGetTickCount();

                    xSemaphoreGive(cacheSemaphore);
                    ESP_LOGI(LOG_TAG, "Word of the day refreshed");
                }
            }
        } else {
            ESP_LOGE(LOG_TAG, "Unexpected response code %i when refreshing", response);
            ESP_LOGE(LOG_TAG, "%s", http.getString());
        }
        vTaskDelay(interval);
    }
}


void wotd_start() {
    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);

    apiKey = prefs_get_string(PREFS_KEY_WORDNIK_APIKEY, String(WORDNIK_API_KEY));

    int interval_minutes = prefs_get_int(PREFS_KEY_WORDNIK_INTERVAL_MINUTES);
    if(interval_minutes == 0) {
        interval_minutes = 120;
        prefs_set_int(PREFS_KEY_WORDNIK_INTERVAL_MINUTES, interval_minutes);
    }
    interval = pdMS_TO_TICKS(interval_minutes * 60 * 1000);

    if(xTaskCreate(
        WordnikTaskFunction,
        "WOTD",
        8000,
        nullptr,
        8,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}