#pragma once

#define TK_TIMEZONE "JST-9"
#define TK_TIMESERVER "pool.ntp.org"
#define TK_SYNC_INTERVAL_SEC 60 * 60

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

/// @brief Start up the timekeeping service
void timekeeping_begin();

/// @brief Get the current time down to the second
tk_time_of_day_t get_current_time_coarse();
/// @brief Get the current time down to the millisecond 
tk_time_of_day_t get_current_time_precise();
/// @brief Get the current date
tk_date_t get_current_date();