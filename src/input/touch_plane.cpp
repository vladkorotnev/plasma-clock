#include <input/touch_plane.h>
#include <map>
#include <device_config.h>
#include <freertos/task.h>
#include <driver/touch_pad.h>
#include <driver/touch_sensor_common.h>
#include <hal/touch_sensor_types.h>
#include <hal/touch_sensor_hal.h>
#include <os_config.h>

// I don't know how this works
// But seems like it works with my janky tinfoil touch screen thingamajiggie

static TaskHandle_t hTask;
static char LOG_TAG[] = "TOUCH";
static std::map<const touch_pad_t, uint16_t> lastReads = {};
static std::map<const touch_pad_t, int> lastDeltas = {};

static esp_err_t init_touch_controller() {
#if HAS(TOUCH_PLANE)
    ESP_LOGI(LOG_TAG, "Initializing");
    esp_err_t rslt;

    rslt = touch_pad_init();
    if(rslt != ESP_OK) {
        ESP_LOGE(LOG_TAG, "touch_pad_init: %s (%i)", esp_err_to_name(rslt), rslt);
        return rslt;
    }
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_SW);

    for(auto &i: HWCONF_TOUCH_PLANE) {
        touch_pad_io_init(i.first);
        rslt = touch_pad_config(i.first, 620);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_config: %s (%i)", esp_err_to_name(rslt), rslt);
            return rslt;
        }

        rslt = touch_pad_set_cnt_mode(i.first, TOUCH_PAD_SLOPE_7, TOUCH_PAD_TIE_OPT_LOW);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_set_cnt_mode: %s (%i)", esp_err_to_name(rslt), rslt);
            return rslt;
        }
    }

    touch_pad_set_voltage(TOUCH_HVOLT_2V6, TOUCH_LVOLT_0V6, TOUCH_HVOLT_ATTEN_0V);

    for(auto &i: HWCONF_TOUCH_PLANE) {
        uint16_t tmp;
        rslt = touch_pad_read(i.first, &tmp);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_read: %s (%i)", esp_err_to_name(rslt), rslt);
            return rslt;
        } else {
            lastReads[i.first] = tmp;
        }
    }
#endif
    return ESP_OK;
}

#if HAS(TOUCH_PLANE)
static void poll_touch_controller() {
    esp_err_t rslt;
    for(auto &i: HWCONF_TOUCH_PLANE) {
        uint16_t tmp;
        rslt = touch_pad_read(i.first, &tmp);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_read_filtered: %s (%i)", esp_err_to_name(rslt), rslt);
            return;
        } else {
            int delta = lastReads[i.first] - tmp;
            //lastDeltas[i.first] = delta;
            if(delta < i.second.release_threshold) {
                // Untouch
                hid_set_key_state(i.second.key, false);
            } else if(delta > i.second.press_threshold) {
                // Touch
                hid_set_key_state(i.second.key, true);
            }
            lastReads[i.first] = tmp;
        }
    }
}

static void touch_task(void* pvParams) {
    while(1) {
        poll_touch_controller();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif

esp_err_t touchplane_start() {
#if HAS(TOUCH_PLANE)
    esp_err_t rslt = init_touch_controller();
    if(rslt == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Creating task");
        xTaskCreate(
            touch_task,
            "TOUCH",
            1024,
            nullptr,
            pisosTASK_PRIORITY_KEYPAD,
            &hTask
        );
    }

    return rslt;
#else
    return ESP_OK;
#endif
}