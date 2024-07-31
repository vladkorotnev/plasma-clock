#include <input/touch_plane.h>
#include <map>
#include <device_config.h>
#include <freertos/task.h>
#include <driver/touch_pad.h>
#include <driver/touch_sensor_common.h>
#include <hal/touch_sensor_types.h>
#include <hal/touch_sensor_hal.h>
#include <console.h>
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

void touchplane_debug(Console * con) {
    while(1) {
       // con->print("R %i U %i\nD %i L %i", lastDeltas[TOUCH_PAD_NUM9], lastDeltas[TOUCH_PAD_NUM8], lastDeltas[TOUCH_PAD_NUM7], lastDeltas[TOUCH_PAD_NUM6]);
        if(hid_test_key_state(KEY_UP)) ESP_LOGI(LOG_TAG, "UP pressed");
        if(hid_test_key_state(KEY_DOWN)) ESP_LOGI(LOG_TAG, "DOWN pressed");
        if(hid_test_key_state(KEY_LEFT)) ESP_LOGI(LOG_TAG, "LEFT pressed");
        if(hid_test_key_state(KEY_RIGHT)) ESP_LOGI(LOG_TAG, "RIGHT pressed");
        if(hid_test_key_state(KEY_HEADPAT)) ESP_LOGI(LOG_TAG, "HEADPAT pressed");

        con->clear();
        if(hid_test_key_state(KEY_UP)) con->print("UP");
        if(hid_test_key_state(KEY_DOWN)) con->print("DOWN");
        if(hid_test_key_state(KEY_LEFT)) con->print("LEFT");
        if(hid_test_key_state(KEY_RIGHT)) con->print("RIGHT");
        if(hid_test_key_state(KEY_HEADPAT)) con->print("HEADPAT");

        vTaskDelay(pdMS_TO_TICKS(100));
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