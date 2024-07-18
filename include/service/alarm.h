#pragma once
#include "time.h"
#include <stdint.h>

#define ALARM_LIST_SIZE 9
#define ALARM_DAY_GLOBAL_ENABLE (1 << 7)
#define ALARM_DAY_OF_WEEK(d) (1 << d)

#define ALARM_ON_DAY(a,d) (((a).days & ALARM_DAY_OF_WEEK(d)) != 0)

typedef struct __attribute__((__packed__)) alarm_setting {
    bool enabled : 1;
    /// @brief LSB is Sunday, B1 is Monday, and so on. Disabled altogether when MSB not set..
    uint8_t days : 7;
    int hour;
    int minute;
    int melody_no;
} alarm_setting_t;

void alarm_init();

const alarm_setting_t * get_alarm_list();
void set_alarm(uint8_t idx, alarm_setting_t setting);

const alarm_setting_t* get_triggered_alarm();
void clear_triggered_alarm();