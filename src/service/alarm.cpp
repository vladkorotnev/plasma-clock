#include <service/alarm.h>
#include <service/time.h>
#include <service/prefs.h>
#include <state.h>

static char LOG_TAG[] = "ALM";

typedef alarm_setting_t alarm_settings_list_t[ALARM_LIST_SIZE];

static TaskHandle_t hTask;
static alarm_settings_list_t alarms = { 0 };
static bool alarm_list_loaded = false;

static tk_time_of_day_t last_alarmed = { 0 };
static tk_date_t last_alarmed_day = { 0 };
static const alarm_setting_t * triggered_alarm = nullptr;

void _load_alarm_list_if_needed() {
    if(!alarm_list_loaded) {
        size_t expected_size = sizeof(alarm_settings_list_t);
        size_t real_size = 0;
        if(!prefs_get_data(PREFS_KEY_ALARM_LIST, &alarms, expected_size, &real_size) || real_size != expected_size) {
            ESP_LOGE(LOG_TAG, "Failed loading alarm list, initialize");
            memset(&alarms, 0, expected_size);
        }
        alarm_list_loaded = true;
    }
}

void _save_alarm_list() {
    prefs_set_data(PREFS_KEY_ALARM_LIST, &alarms, sizeof(alarm_settings_list_t));
}

const alarm_setting_t * get_alarm_list() {
    return &alarms[0];
}

void set_alarm(uint8_t idx, alarm_setting_t setting) {
    if(idx >= ALARM_LIST_SIZE) return;
    _load_alarm_list_if_needed();
    alarms[idx] = setting;
    _save_alarm_list();
    ESP_LOGI(LOG_TAG, "Set alarm %i: days 0x%2.x, melody %i, %2.i:%2.i", idx, setting.days, setting.melody_no, setting.hour, setting.minute);
}

const alarm_setting_t* get_triggered_alarm() {
    return triggered_alarm;
}

void clear_triggered_alarm() {
    triggered_alarm = nullptr;
}

static void alarm_task(void*) {
    ESP_LOGI(LOG_TAG, "Task started");
    while(1) {
        tk_time_of_day_t now;
        tk_date today;
        today = get_current_date();
        now = get_current_time_coarse();
        now.millisecond = 0;
        now.second = 0;

        if(memcmp(&now, &last_alarmed, sizeof(tk_time_of_day_t)) == 0 &&
            memcmp(&today, &last_alarmed_day, sizeof(tk_date_t)) == 0) {
            // already alarmed at this time
            continue;
        }

        for(int i = 0; i < ALARM_LIST_SIZE; i++) {
            alarm_setting_t alarm = alarms[i];
            if(alarm.enabled) {
                if(alarm.days == 0 || ALARM_ON_DAY(alarm, today.dayOfWeek)){
                    if(alarm.hour == now.hour && alarm.minute == now.minute) {
                        ESP_LOGI(LOG_TAG, "Triggering alarm at index %i", i);
                        triggered_alarm = &alarms[i];
                        memcpy(&last_alarmed, &now, sizeof(tk_time_of_day_t));
                        memcpy(&last_alarmed_day, &today, sizeof(tk_date_t));

                        if(alarm.days == 0) {
                            ESP_LOGI(LOG_TAG, "Alarm %i was singular, turning OFF!", i);
                            alarm.days = 0;
                            set_alarm(i, alarm);
                        }

                        push_state(STATE_ALARMING, TRANSITION_WIPE);
                        break;
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void alarm_init() {
    _load_alarm_list_if_needed();
    xTaskCreate(
        alarm_task,
        "ALARM",
        4096,
        nullptr,
        4,
        &hTask
    );
}