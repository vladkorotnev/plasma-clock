#pragma once
#include <ctime>

#define TK_TIMEZONE "JST-9"
#define TK_TIMESERVER "pool.ntp.org"
#define TK_SYNC_INTERVAL_SEC 60 * 60

typedef struct tk_time_of_day {
    int hour;
    int minute;
    int second;
    int millisecond;
} tk_time_of_day_t;

static const tk_time_of_day_t ONE_HOUR = { .hour = 1, .minute = 0, .second = 0, .millisecond = 0 };

typedef struct tk_date {
    int year;
    int month;
    int day;
    /// @brief 0 is Sunday, 1 is Monday and so on...
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
/// @brief Set the current time without changing the timezone et al
void set_current_time(tk_time_of_day_t);
/// @brief Set the current date without changing the timezone et al
void set_current_date(tk_date_t);
/// @brief Returns the system uptime as human-readable components
tk_time_of_day_t get_uptime();

tk_date_t unixtime_to_date(time_t unixtime);
tk_time_of_day_t unixtime_to_time(time_t unixtime);

tk_time_of_day_t operator -(const tk_time_of_day_t& a, const tk_time_of_day_t& b);
bool operator==(const tk_time_of_day_t& a, const tk_time_of_day_t& b);
bool operator<(const tk_time_of_day_t& a, const tk_time_of_day_t& b);
bool operator<=(const tk_time_of_day_t& a, const tk_time_of_day_t& b);
bool operator>(const tk_time_of_day_t& a, const tk_time_of_day_t& b);
bool operator>=(const tk_time_of_day_t& a, const tk_time_of_day_t& b);

void convert_to_12h(tk_time_of_day_t * time, bool * out_pm_flag = nullptr);
bool time_in_range(const tk_time_of_day_t& time, const tk_time_of_day_t& start, const tk_time_of_day_t& end);
