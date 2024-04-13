#include <service/time.h>
#include <service/prefs.h>
#include "esp_sntp.h"

void timekeeping_begin() {
    if(sntp_enabled()){
        sntp_stop();
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, prefs_get_string(PREFS_KEY_TIMESERVER, String(TK_TIMESERVER)).c_str());
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    int sync_interval = prefs_get_int(PREFS_KEY_TIME_SYNC_INTERVAL_SEC);
    if(sync_interval == 0) {
        sync_interval = TK_SYNC_INTERVAL_SEC;
        prefs_set_int(PREFS_KEY_TIME_SYNC_INTERVAL_SEC, sync_interval);
    }
    sntp_set_sync_interval(sync_interval * 1000);
    sntp_init();

    String tz = prefs_get_string(PREFS_KEY_TIMEZONE, String(TK_TIMEZONE));
    setenv("TZ", tz.c_str(), 1);
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