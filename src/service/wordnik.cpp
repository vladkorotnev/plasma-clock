#include <device_config.h>
#include <service/wordnik.h>
#include <os_config.h>

#if HAS(WORDNIK_API)
#include <service/prefs.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static char LOG_TAG[] = "WORDNIK";

static const char * currentApi = "https://api.wordnik.com/v4/words.json/wordOfTheDay?api_key=%s";
EXT_RAM_ATTR static String apiKey;
static TickType_t interval;

static TaskHandle_t hTask = NULL;
static bool hasQueries = false;
static SemaphoreHandle_t firstRunSemaphore;
static SemaphoreHandle_t cacheSemaphore;
EXT_RAM_ATTR static char wordCache[128];
EXT_RAM_ATTR static char definitionCache[256];
static TickType_t lastUpdate = 0;

TickType_t wotd_get_last_update() {
    return lastUpdate;
}

bool wotd_get_current(char * w, size_t wsz, char * d, size_t dsz) {
    if(w == nullptr || d == nullptr) return false;

    if(!xSemaphoreTake(cacheSemaphore, pdMS_TO_TICKS(1000))) {
        ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
        return false;
    }

    strncpy(w, wordCache, wsz-1);
    strncpy(d, definitionCache, dsz-1);
    hasQueries = true;
    xSemaphoreGive(cacheSemaphore);
    return true;
}

void WordnikTaskFunction( void * pvParameter )
{
    ESP_LOGI(LOG_TAG, "Task started");

    EXT_RAM_ATTR static WiFiClientSecure client;
    EXT_RAM_ATTR static HTTPClient http;

    EXT_RAM_ATTR static char url[128];
    snprintf(url, 150, currentApi, apiKey.c_str());
    bool isFailure = false;

    while(1) {
        // If there were no more queries since the last update, supposedly nobody is actually using the service, so we don't really need to update it
        if(hasQueries) {
            isFailure = false;
            http.begin(client, url);
            client.setInsecure();
            ESP_LOGV(LOG_TAG, "Query: %s", url);
            int response = http.GET();
            if(response == HTTP_CODE_OK) {
                EXT_RAM_ATTR static JsonDocument response;

                DeserializationError error = deserializeJson(response, http.getStream());

                if (error) {
                    ESP_LOGE(LOG_TAG, "Parse error: %s", error.c_str());
                    isFailure = true;
                } else {
                    if(!xSemaphoreTake(cacheSemaphore, pdMS_TO_TICKS(5000))) {
                        ESP_LOGE(LOG_TAG, "Timeout waiting on cache semaphore");
                        isFailure = true;
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
                isFailure = true;
            }
            client.stop();
            if(!isFailure) {
                hasQueries = false;
            }
            xSemaphoreGive(firstRunSemaphore);
            vTaskDelay(isFailure ? pdMS_TO_TICKS(10000) : interval);
        }
    }
}


void wotd_start() {
    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);

    firstRunSemaphore = xSemaphoreCreateBinary();
    hasQueries = true;

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
        pisosTASK_PRIORITY_WORDNIK,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    } else {
        // avoid core contention by waiting for the first request to go through before continuing with boot
        ESP_LOGI(LOG_TAG, "Waiting on first run semaphore");
        xSemaphoreTake(firstRunSemaphore, portMAX_DELAY);
    }
}

#else
void wotd_start() { }
bool wotd_get_current(char * word, size_t word_sz, char * definition, size_t def_sz) { return false; }
TickType_t wotd_get_last_update() { return 0; }
#endif