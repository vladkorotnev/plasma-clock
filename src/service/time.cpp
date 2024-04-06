#include <service/time.h>
#include "esp_sntp.h"

void timekeeping_begin() {
    if(sntp_enabled()){
        sntp_stop();
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, TK_TIMESERVER);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_set_sync_interval(TK_SYNC_INTERVAL);
    sntp_init();

    setenv("TZ", TK_TIMEZONE, 1);
    tzset();
}

tk_time_of_day_t get_current_time_coarse() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    return tk_time_of_day_t {
        .hour = timeinfo.tm_hour,
        .minute = timeinfo.tm_min,
        .second = timeinfo.tm_sec,
        .millisecond = 0
    };
}
tk_time_of_day_t get_current_time_precise() {
    tk_time_of_day_t time = get_current_time_coarse();

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    time.millisecond = tv_now.tv_usec / 1000;

    return time;
}

tk_date_t get_current_date() {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    return tk_date_t {
        .year = timeinfo.tm_year,
        .month = timeinfo.tm_mon,
        .day = timeinfo.tm_mday,
        .dayOfWeek = timeinfo.tm_wday
    };
}