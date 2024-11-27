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

#if HAS(IR_RECEIVER)
static IRrecv receiver(HWCONF_IR_RECV_GPIO);

static void ir_task(void*) {
    static decode_results results;
    while(1) {
        if(receiver.decode(&results)) {
            ESP_LOGI(LOG_TAG, "IR code TYPE=%i, VALUE=%i, ADDRESS=%i, COMMAND=%i", results.decode_type, results.value, results.address, results.command);

            for(auto &mapping: HWCONF_IR_BUTTONS) {
                const infrared_identifier_t * id = &mapping.first;
                if(id->protocol == results.decode_type &&
                   id->address == results.address &&
                   id->command == results.command &&
                   id->value == results.value) {

                    if(last_pressed != KEY_MAX_INVALID) {
                        if(last_pressed == mapping.second)
                            break;
                        hid_set_key_state(last_pressed, false);
                    }

                    last_pressed = mapping.second;
                    hid_set_key_state(mapping.second, true);
                }
            }
        } else {
            if(last_pressed != KEY_MAX_INVALID) {
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
            4096,
            nullptr,
            pisosTASK_PRIORITY_KEYPAD,
            &hTask
        );
#endif
}