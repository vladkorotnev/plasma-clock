#include <service/power_management.h>
#include <Arduino.h>

static char LOG_TAG[] = "PM";
static TaskHandle_t hTask = NULL;

static TickType_t lastMotionTime;
static bool isHvOff;
static bool isDisplayOff;

static SensorPool * sensors;
static PlasmaDisplayIface * display;
static Beeper * beeper;

static int lastLightness;
static bool isBright;

extern "C" void PMTaskFunction( void * pvParameter );

void PMTaskFunction( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "I'm here to chew gum and switch your display power, and looks like I'm all out of gum!");

    lastMotionTime = xTaskGetTickCount();
    isBright = true;
    isDisplayOff = false;
    isHvOff = false;
    static TickType_t now;


    while(1) {
        now = xTaskGetTickCount();
        // Adjust dimmer according to ambient light
        sensor_info_t * light_info = sensors->get_info(SENSOR_ID_AMBIENT_LIGHT);
        if(light_info != nullptr) {
            int current_lightness = light_info->last_result;
            if(current_lightness > PM_LIGHTNESS_THRESH_UP && current_lightness > lastLightness && !isBright) {
                ESP_LOGI(LOG_TAG, "Changing to bright mode");
                isBright = true;
                display->set_bright(true);
            } 
            else if(current_lightness < PM_LIGHTNESS_THRESH_DOWN && current_lightness < lastLightness && isBright) {
                ESP_LOGI(LOG_TAG, "Changing to dim mode");
                isBright = false;
                display->set_bright(false);
            }

            lastLightness = light_info->last_result;
        }

        // Turn off display when no motion in the room for a while
        sensor_info_t * motion_info = sensors->get_info(SENSOR_ID_MOTION);
        if(motion_info != nullptr) {
            if(motion_info->last_result > 0) {
                // There was some motion, reenable the display
                if(isDisplayOff) {
                    ESP_LOGI(LOG_TAG, "Start display");
                    display->set_show(true);
                    beeper->set_channel_state(CHANNEL_AMBIANCE, true);
                    isDisplayOff = false;
                }

                if(isHvOff) {
                    // Sometimes dim mode voltage is not enough, so enable bright mode when turning HV back on for a bit
                    ESP_LOGV(LOG_TAG, "Reenable HV");
                    display->set_bright(true);
                    display->set_power(true);
                    isHvOff = false;
                    vTaskDelay(pdMS_TO_TICKS(500));
                    display->set_bright(isBright);
                }

                lastMotionTime = now;
            } else {
              if(now - lastMotionTime >= pdMS_TO_TICKS(PM_MOTIONLESS_TURN_OFF_DELAY) && !isDisplayOff) {
                // No motion for a while, turn off display, first only logically
                ESP_LOGI(LOG_TAG, "Stop display");
                display->set_show(false);
                beeper->set_channel_state(CHANNEL_AMBIANCE, false);
                isDisplayOff = true;
              } 

              if(now - lastMotionTime >= pdMS_TO_TICKS(PM_MOTIONLESS_TURN_OFF_DELAY) && !isHvOff) {
                // No motion for a really long while, shut down the HV power supply
                ESP_LOGI(LOG_TAG, "Stop HV");
                display->set_power(false);
                isHvOff = true;
              } 
            }
        }

        vTaskDelay(PM_INTERVAL);
    }
}

void power_mgmt_start(SensorPool * s, PlasmaDisplayIface * d, Beeper * b) {
    sensors = s;
    display = d;
    beeper = b;
    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        PMTaskFunction,
        "PM",
        4096,
        nullptr,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void power_mgmt_pause() {
    if(hTask == NULL) return;
    ESP_LOGI(LOG_TAG, "Pause");
    vTaskSuspend(hTask);
    display->set_power(true);
    display->set_bright(true);
    display->set_show(true);
    beeper->set_channel_state(CHANNEL_AMBIANCE, true);
}

void power_mgmt_resume() {
    if(hTask == NULL) return;
    ESP_LOGI(LOG_TAG, "Resume");
    vTaskResume(hTask);
    display->set_power(!isHvOff);
    display->set_bright(!isBright);
    display->set_show(!isDisplayOff);
    beeper->set_channel_state(CHANNEL_AMBIANCE, !isDisplayOff);
}