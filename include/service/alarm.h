#pragma once
#include "time.h"
#include <stdint.h>

#define ALARM_LIST_SIZE 10
#define ALARM_DAY_GLOBAL_ENABLE (1 << 7)
#define ALARM_DAY_OF_WEEK(d) (1 << d)

#define ALARM_ON_DAY(a,d) ((a.days & ALARM_DAY_OF_WEEK(d)) != 0)
#define ALARM_IS_ENABLED(x) ((x.days & ALARM_DAY_GLOBAL_ENABLE) != 0)

typedef struct alarm_setting {
    /// @brief LSB is Sunday, B1 is Monday, and so on. Disabled altogether when MSB not set..
    uint8_t days;
    int hour;
    int minute;
    int melody_no;
} alarm_setting_t;

typedef alarm_setting alarm_settings_list_t[ALARM_LIST_SIZE];

void alarm_init();

const alarm_settings_list_t * get_alarm_list();
void set_alarm(uint8_t idx, alarm_setting_t setting);

const alarm_setting_t* get_triggered_alarm();
void clear_triggered_alarm();