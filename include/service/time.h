#pragma once

#define TK_TIMEZONE "JST-9"
#define TK_TIMESERVER "pool.ntp.org"
#define TK_SYNC_INTERVAL 60 * 60 * 1000

typedef struct tk_time_of_day {
    int hour;
    int minute;
    int second;
    int millisecond;
} tk_time_of_day_t;

typedef struct tk_date {
    int year;
    int month;
    int day;
    int dayOfWeek;
} tk_date_t;

void timekeeping_begin();

tk_time_of_day_t get_current_time_coarse();
tk_time_of_day_t get_current_time_precise();
tk_date_t get_current_date();