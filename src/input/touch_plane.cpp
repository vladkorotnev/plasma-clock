#include <input/touch_plane.h>
#include <map>
#include <device_config.h>
#include <freertos/task.h>
#include <driver/touch_pad.h>
#include <driver/touch_sensor_common.h>
#include <hal/touch_sensor_types.h>
#include <hal/touch_sensor_hal.h>
#include <console.h>

static TaskHandle_t hTask;
static char LOG_TAG[] = "TOUCH";
static std::map<const touch_pad_t, uint16_t> lastReads = {};
static std::map<const touch_pad_t, uint16_t> maxReads = {};

static esp_err_t init_touch_controller() {
#if HAS(TOUCH_PLANE)
    ESP_LOGI(LOG_TAG, "Initializing");
    esp_err_t rslt;

    rslt = touch_pad_init();
    if(rslt != ESP_OK) {
        ESP_LOGE(LOG_TAG, "touch_pad_init: %s (%i)", esp_err_to_name(rslt), rslt);
        return rslt;
    }

    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);

    for(auto &i: HWCONF_TOUCH_PLANE) {
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

    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    touch_pad_filter_start(200);

    for(auto &i: HWCONF_TOUCH_PLANE) {
        uint16_t tmp;
        rslt = touch_pad_read_filtered(i.first, &tmp);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_read_filtered: %s (%i)", esp_err_to_name(rslt), rslt);
            return rslt;
        } else {
            lastReads[i.first] = tmp;
            maxReads[i.first] = tmp;
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
        rslt = touch_pad_read_filtered(i.first, &tmp);
        if(rslt != ESP_OK) {
            ESP_LOGE(LOG_TAG, "touch_pad_read_filtered: %s (%i)", esp_err_to_name(rslt), rslt);
            return;
        } else {
            
            uint16_t thresh = 15;
            uint16_t false_thresh = 30;

            int delta = lastReads[i.first] - tmp;
            if(abs(delta) >= thresh && abs(delta) < false_thresh) {
                if(delta < 0) {
                    // Reduction of value: press
                    hid_set_key_state(i.second, true);
                    ESP_LOGI(LOG_TAG, "Press button %i (%i)", i.first, i.second);
                } else {
                    // Increment of value: release
                    hid_set_key_state(i.second, false);
                    ESP_LOGI(LOG_TAG, "Release button %i (%i)", i.first, i.second);
                }
            }

            lastReads[i.first] = tmp;
        }
    }
}

static void touch_task(void* pvParams) {
    while(1) {
        poll_touch_controller();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void touchplane_debug(Console * con) {
    while(1) {
        con->print("R %i U %i\nD %i L %i", lastReads[TOUCH_PAD_NUM9], lastReads[TOUCH_PAD_NUM8], lastReads[TOUCH_PAD_NUM7], lastReads[TOUCH_PAD_NUM6]);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
#endif

esp_err_t start_touchplane_scan() {
#if HAS(TOUCH_PLANE)
    esp_err_t rslt = init_touch_controller();
    if(rslt == ESP_OK) {
        ESP_LOGI(LOG_TAG, "Creating task");
        xTaskCreate(
            touch_task,
            "TOUCH",
            4096,
            nullptr,
            configMAX_PRIORITIES - 1,
            &hTask
        );
    }

    return rslt;
#else
    return ESP_OK;
#endif
}