#include <input/keypad.h>
#include <device_config.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "KEYP";

static TaskHandle_t hTask;

static void keypad_task(void*) {
    while(1) {
        for(auto i: HWCONF_KEYPAD) {
            int lvl = gpio_get_level(i.first);
            hid_set_key_state(i.second, lvl > 0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void keypad_start() {
#if HAS(KEYPAD)
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = gpio_pullup_t::GPIO_PULLUP_DISABLE,
        .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE
    };

    for(auto i: HWCONF_KEYPAD) {
        io_conf.pin_bit_mask |= 1ULL << i.first;
    }

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(LOG_TAG, "Creating task");
        xTaskCreate(
            keypad_task,
            "KEYPAD",
            4096,
            nullptr,
            configMAX_PRIORITIES - 3,
            &hTask
        );
#endif
}