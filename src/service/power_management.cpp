#include <service/power_management.h>
#include <service/prefs.h>
#include <input/keys.h>
#include <Arduino.h>
#include <os_config.h>

static char LOG_TAG[] = "PM";
static TaskHandle_t hTask = NULL;

static TickType_t lastMotionTime;
static bool isHvOff = false;
static bool isDisplayOff = false;

static SensorPool * sensors;
static DisplayDriver * display;
static Beeper * beeper;

static TimerSensor * startled_sensor;

static int lastLightness;
static bool isBright = true;

static bool noSoundWhenOff;
static int motionlessTimeOff;
static int motionlessTimeHvOff;
static int lightnessThreshUp;
static int lightnessThreshDown;

extern "C" void PMTaskFunction( void * pvParameter );

static void wake_up(TickType_t now) {
    // There was some motion, reenable the display
    if(isDisplayOff) {
        ESP_LOGI(LOG_TAG, "Start display");
        display->set_show(true);
        beeper->set_channel_state(CHANNEL_AMBIANCE, true);
        isDisplayOff = false;
        startled_sensor->trigger();
    }

    if(isHvOff) {
        // Sometimes dim mode voltage is not enough, so enable bright mode when turning HV back on for a bit
        ESP_LOGV(LOG_TAG, "Reenable HV");
    #if HAS(VARYING_BRIGHTNESS)
        display->set_bright(true);
    #endif
        display->set_power(true);
        isHvOff = false;
        vTaskDelay(pdMS_TO_TICKS(500));
    #if HAS(VARYING_BRIGHTNESS)
        display->set_bright(isBright);
    #endif
    }

    // cannot use getTickCount here, because then lastMotionTime will be in the future
    // and thus further conditions will think it's way off in the past (due to ticktype_t being unsigned)
    // which causes blinking when waking up the device with buttons without triggering motion somehow
    lastMotionTime = now;
}

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

        #if HAS(VARYING_BRIGHTNESS) && HAS(LIGHT_SENSOR)
        // Adjust dimmer according to ambient light
        sensor_info_t * light_info = sensors->get_info(SENSOR_ID_AMBIENT_LIGHT);
        if(light_info != nullptr) {
            int current_lightness = light_info->last_result;
            if(current_lightness > lightnessThreshUp && current_lightness > lastLightness && !isBright) {
                ESP_LOGI(LOG_TAG, "Changing to bright mode");
                isBright = true;
                display->set_bright(true);
            } 
            else if(current_lightness < lightnessThreshDown && current_lightness < lastLightness && isBright) {
                ESP_LOGI(LOG_TAG, "Changing to dim mode");
                isBright = false;
                display->set_bright(false);
            }

            lastLightness = light_info->last_result;
        }
        #endif

        sensor_info_t * hid_info = sensors->get_info(VIRTSENSOR_ID_HID_STARTLED);
        if(hid_info) {
            // wake up on keypress
            if(hid_info->last_result > 0) {
                wake_up(now);
            }
        }

        #if HAS(MOTION_SENSOR)
        // Turn off display when no motion in the room for a while
        sensor_info_t * motion_info = sensors->get_info(SENSOR_ID_MOTION);
        if(motion_info != nullptr) {
            if(motion_info->last_result > 0) {
                // There was some motion, reenable the display
                wake_up(now);
            } else {
              if(now - lastMotionTime >= pdMS_TO_TICKS(motionlessTimeOff) && !isDisplayOff) {
                // No motion for a while, turn off display, first only logically
                ESP_LOGI(LOG_TAG, "Stop display");
                display->set_show(false);
                if(noSoundWhenOff) {
                    beeper->set_channel_state(CHANNEL_AMBIANCE, false);
                }
                isDisplayOff = true;
              } 

              if(now - lastMotionTime >= pdMS_TO_TICKS(motionlessTimeHvOff) && !isHvOff) {
                // No motion for a really long while, shut down the HV power supply
                ESP_LOGI(LOG_TAG, "Stop HV");
                display->set_power(false);
                isHvOff = true;
              } 
            }
        }
        #endif

        vTaskDelay(PM_INTERVAL);
    }
}

void power_mgmt_start(SensorPool * s, DisplayDriver * d, Beeper * b) {
    sensors = s;
    display = d;
    beeper = b;

    startled_sensor = new TimerSensor(pdMS_TO_TICKS(5000));
    sensors->add(VIRTSENSOR_ID_PMU_STARTLED, startled_sensor, SENSOR_POLLRATE_ASARP);

    noSoundWhenOff = prefs_get_bool(PREFS_KEY_NO_SOUND_WHEN_OFF);
    
    motionlessTimeOff = prefs_get_int(PREFS_KEY_MOTIONLESS_TIME_OFF_SECONDS) * 1000;
    if(motionlessTimeOff == 0) motionlessTimeOff = PM_MOTIONLESS_TURN_OFF_DELAY;

    motionlessTimeHvOff = prefs_get_int(PREFS_KEY_MOTIONLESS_TIME_HV_OFF_SECONDS) * 1000;
    if(motionlessTimeHvOff == 0) motionlessTimeHvOff = PM_MOTIONLESS_HV_SHUTDOWN_DELAY;
    motionlessTimeHvOff += motionlessTimeOff;

    lightnessThreshDown = prefs_get_int(PREFS_KEY_LIGHTNESS_THRESH_DOWN);
    lightnessThreshUp = prefs_get_int(PREFS_KEY_LIGHTNESS_THRESH_UP);

    ESP_LOGI(LOG_TAG, "MotionlessTime %i, HvOffTime %i, LightThresh %i %i", motionlessTimeOff, motionlessTimeHvOff, lightnessThreshDown, lightnessThreshUp);

#if !HAS(MOTION_SENSOR) && !HAS(LIGHT_SENSOR)
    ESP_LOGE(LOG_TAG, "Build without light and motion sensor support: cannot start!");
    return; // useless without a light and motion sensor
#endif

    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        PMTaskFunction,
        "PM",
        4096,
        nullptr,
        pisosTASK_PRIORITY_POWER_MANAGEMENT,
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
#if HAS(VARYING_BRIGHTNESS)
    display->set_bright(true);
#endif
    display->set_show(true);
    beeper->set_channel_state(CHANNEL_AMBIANCE, true);
}

void power_mgmt_resume() {
    if(hTask == NULL) return;
    ESP_LOGI(LOG_TAG, "Resume");
    vTaskResume(hTask);
    display->set_power(!isHvOff);
#if HAS(VARYING_BRIGHTNESS)
    display->set_bright(isBright);
#endif
    display->set_show(!isDisplayOff);
    beeper->set_channel_state(CHANNEL_AMBIANCE, !isDisplayOff);
}