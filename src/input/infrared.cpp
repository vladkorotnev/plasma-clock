#include "input/infrared.h"
#include <device_config.h>
#include <esp32-hal-log.h>
#include <map>
#include <os_config.h>

#if HAS(IR_RECEIVER)
// NB: disable all the stuff of no use in the platformio.ini file!
#include <IRrecv.h>
#endif

static char LOG_TAG[] = "IRRC";

static TaskHandle_t hTask;

static key_id_t last_pressed = KEY_MAX_INVALID;
static TickType_t last_pressed_timestamp = 0;

#if HAS(IR_RECEIVER)
static IRrecv receiver(HWCONF_IR_RECV_GPIO);

static void ir_task(void*) {
    static decode_results results;
    while(1) {
        if(receiver.decode(&results)) {
            if(results.repeat) {
                last_pressed_timestamp = xTaskGetTickCount();
            } else {
                bool found = false;
                for(int i = 0; i < sizeof(HWCONF_IR_BUTTONS) / sizeof(infrared_identifier_t); i++) {
                    const infrared_identifier_t * id = &HWCONF_IR_BUTTONS[i];
                    ESP_LOGV(LOG_TAG, "Check against TYPE=%i, VALUE=0x%x, ADDRESS=0x%x, COMMAND=0x%x", id->protocol, id->value, id->address, id->command);
                    if(id->protocol == (int) results.decode_type &&
                    id->address == results.address &&
                    id->command == results.command &&
                    id->value == results.value) {
                        found = true;
                        last_pressed_timestamp = xTaskGetTickCount();

                        if(last_pressed != KEY_MAX_INVALID) {
                            if(last_pressed == id->key)
                                break;
                            ESP_LOGI(LOG_TAG, "Unpress key %i (recv)", last_pressed);
                            hid_set_key_state(last_pressed, false);
                        }

                        last_pressed = id->key;
                        ESP_LOGI(LOG_TAG, "Press key %i", last_pressed);
                        hid_set_key_state(id->key, true);
                    }
                }
                if(!found) {
                    ESP_LOGI(LOG_TAG, "IR code { .protocol = %i, .address = 0x%x, .command = 0x%x, .value = 0x%x, .key = ? }", results.decode_type, results.address, results.command, results.value);
                }
            }

            receiver.resume();
        } else {
            if(last_pressed != KEY_MAX_INVALID && (xTaskGetTickCount() - last_pressed_timestamp) > pdMS_TO_TICKS(150)) {
                ESP_LOGI(LOG_TAG, "Unpress key %i (timeout)", last_pressed);
                hid_set_key_state(last_pressed, false);
                last_pressed = KEY_MAX_INVALID;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#endif

void infrared_start() {
#if HAS(IR_RECEIVER)
    receiver.enableIRIn(true);

    ESP_LOGI(LOG_TAG, "Creating task");
        xTaskCreate(
            ir_task,
            "IRRC",
            8000,
            nullptr,
            pisosTASK_PRIORITY_KEYPAD,
            &hTask
        );
#endif
}