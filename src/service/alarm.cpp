#include <service/alarm.h>
#include <service/time.h>
#include <service/prefs.h>
#include <state.h>
#include <os_config.h>

static char LOG_TAG[] = "ALM";

typedef alarm_setting_t alarm_settings_list_t[ALARM_LIST_SIZE];

static TaskHandle_t hTask;
static alarm_settings_list_t alarms = { 0 };
static bool alarm_list_loaded = false;
static SensorPool * sensors = nullptr;

static alarm_setting_t last_alarmed = { 0 }; // <- if we were to just save the index, reusing the same alarm slot on the same day would not work.
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
    ESP_LOGI(LOG_TAG, "Set alarm %i: days 0x%2.x, melody %i, %2.i:%2.i, smart %i (margin %i)", idx, setting.days, setting.melody_no, setting.hour, setting.minute, setting.smart, setting.smart_margin_minutes);
}

const alarm_setting_t* get_triggered_alarm() {
    return triggered_alarm;
}

void clear_triggered_alarm() {
    triggered_alarm = nullptr;
}

static void _trigger_alarm_if_needed(tk_date& today, int index) {
    alarm_setting_t alarm = alarms[index];
    // a hash is probably in order here, but CBA.
    if(memcmp(&alarm, &last_alarmed, sizeof(alarm_setting_t)) == 0 &&
        memcmp(&today, &last_alarmed_day, sizeof(tk_date_t)) == 0) {
        // already alarmed this alarm today, nothing to do
        return;
    }

    ESP_LOGI(LOG_TAG, "Triggering alarm at index %i", index);
    triggered_alarm = &alarms[index];
    memcpy(&last_alarmed, &alarm, sizeof(alarm_setting_t));
    memcpy(&last_alarmed_day, &today, sizeof(tk_date_t));

    if(alarm.days == 0) {
        ESP_LOGI(LOG_TAG, "Alarm %i was singular, turning OFF!", index);
        alarm.enabled = false;
        set_alarm(index, alarm);
    }

    push_state(STATE_ALARMING, TRANSITION_WIPE);
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

        for(int i = 0; i < ALARM_LIST_SIZE; i++) {
            alarm_setting_t alarm = alarms[i];
            tk_time_of_day_t alarm_time = { .hour = alarm.hour, .minute = alarm.minute, .second = 0, .millisecond = 0 };
            if(alarm.enabled) {
                if(alarm.days == 0 || ALARM_ON_DAY(alarm, today.dayOfWeek)){
                    if(now == alarm_time) {
                        ESP_LOGI(LOG_TAG, "Triggering alarm %i due to TIME reasons", i);
                        _trigger_alarm_if_needed(today, i);
                        break;
                    } else if(alarm.smart && alarm.smart_margin_minutes > 0
                        && sensors && sensors->exists(SENSOR_ID_MOTION)) {
                        // Simple smart alarm logic based on motion sensing
                        tk_time_of_day_t margin_as_time = { .hour = 0, .minute = alarm.smart_margin_minutes, .second = 0, .millisecond = 0 };
                        tk_time_of_day_t earliest_time = alarm_time - margin_as_time;
                        if ((now - earliest_time <= ONE_HOUR) && (alarm_time - now <= ONE_HOUR)) {
                            // We are now within the alarm margin, so in "armed" state
                            sensor_info_t * motn = sensors->get_info(SENSOR_ID_MOTION);
                            if(motn) {
                                ESP_LOGV(LOG_TAG, "Alarm %i is ARMED for smart mode", i);
                                // NB: check if last motion sensor poll was recent enough?
                                if(motn->last_result) {
                                    // there was motion, kick off the alarm
                                    ESP_LOGI(LOG_TAG, "Triggering alarm %i due to MOTION reasons", i);
                                    _trigger_alarm_if_needed(today, i);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

const alarm_setting_t* get_upcoming_alarm() {
    _load_alarm_list_if_needed();

    tk_time_of_day_t now = get_current_time_coarse();
    tk_date today = get_current_date();

    tk_time_of_day_t next_alarm_time = { .hour = INT_MAX, .minute = INT_MAX, .second = INT_MAX, .millisecond = INT_MAX };
    const alarm_setting_t * next_alarm = nullptr;

    // Look for any alarms in today
    for (int i = 0; i < ALARM_LIST_SIZE; i++) {
        const alarm_setting_t * alarm = &alarms[i];
        if (alarm->enabled) {
            if (alarm->days == 0 || ALARM_ON_DAY(*alarm, today.dayOfWeek)) {
                tk_time_of_day_t alarm_time = { .hour = alarm->hour, .minute = alarm->minute, .second = 0, .millisecond = 0 };
                if (alarm_time > now) {
                    if (alarm_time < next_alarm_time) {
                        next_alarm_time = alarm_time;
                        next_alarm = alarm;
                    }
                }
            }
        }
    }

    return next_alarm;
}

void alarm_init(SensorPool* s) {
    sensors = s;
    _load_alarm_list_if_needed();
    xTaskCreate(
        alarm_task,
        "ALARM",
        4096,
        nullptr,
        pisosTASK_PRIORITY_ALARM,
        &hTask
    );
}