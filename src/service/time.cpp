#include <service/time.h>
#include <service/prefs.h>
#include <esp_sntp.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "TIME";
static char * ntp_server = nullptr;

void timekeeping_begin() {
    if(prefs_get_bool(PREFS_KEY_TIMESERVER_ENABLE)) {
        if(sntp_enabled()){
            sntp_stop();
        }

        sntp_setoperatingmode(SNTP_OPMODE_POLL);

        String timeServer = prefs_get_string(PREFS_KEY_TIMESERVER, String(TK_TIMESERVER));
        // Looks like the SNTP API expects the server name to be by reference, so if the String gets released by the end of this function,
        // the sync will not work. Let's copy it to our own static memory.
        if(ntp_server != nullptr) free(ntp_server);

        size_t tmpLen = timeServer.length();
        ntp_server = (char*) malloc(tmpLen + 1);
        memcpy(ntp_server, timeServer.c_str(), tmpLen);
        ntp_server[tmpLen] = 0x0;

        ESP_LOGI(LOG_TAG, "Starting NTP service: %s", ntp_server);

        sntp_setservername(0, ntp_server);
        sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

        int sync_interval = prefs_get_int(PREFS_KEY_TIME_SYNC_INTERVAL_SEC);
        if(sync_interval == 0) {
            sync_interval = TK_SYNC_INTERVAL_SEC;
            prefs_set_int(PREFS_KEY_TIME_SYNC_INTERVAL_SEC, sync_interval);
        }
        ESP_LOGI(LOG_TAG, "Sync interval: %i", sync_interval);
        sntp_set_sync_interval(sync_interval * 1000);
        sntp_init();
    } else {
        ESP_LOGW(LOG_TAG, "NTP is disabled!");
    }

    String tz = prefs_get_string(PREFS_KEY_TIMEZONE, String(TK_TIMEZONE));
    ESP_LOGI(LOG_TAG, "Timezone: %s", tz.c_str());
    setenv("TZ", tz.c_str(), 1);
    tzset();
}

tk_date_t unixtime_to_date(time_t unixtime) {
    struct tm timeinfo;
    localtime_r(&unixtime, &timeinfo);
    return tk_date_t {
        .year = timeinfo.tm_year + 1900,
        .month = timeinfo.tm_mon + 1,
        .day = timeinfo.tm_mday,
        .dayOfWeek = timeinfo.tm_wday
    };
}

tk_time_of_day_t unixtime_to_time(time_t unixtime) {
    struct tm timeinfo;
    localtime_r(&unixtime, &timeinfo);
    return tk_time_of_day_t {
        .hour = timeinfo.tm_hour,
        .minute = timeinfo.tm_min,
        .second = timeinfo.tm_sec,
        .millisecond = 0
    };
}

tk_time_of_day_t get_current_time_coarse() {
    time_t now;
    time(&now);
    return unixtime_to_time(now);
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
    time(&now);
    return unixtime_to_date(now);
}

void set_current_time(tk_time_of_day_t new_time) {
    tk_date_t date = get_current_date();
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    t.tm_year = date.year - 1900;
    t.tm_mon = date.month - 1;
    t.tm_mday = date.day;
    t.tm_hour = new_time.hour;
    t.tm_min = new_time.minute;
    t.tm_sec = new_time.second;
    time_t timeSinceEpoch = mktime(&t);

    struct timeval new_time_tv;
    new_time_tv.tv_sec = timeSinceEpoch;
    new_time_tv.tv_usec = new_time.millisecond * 1000;
    settimeofday(&new_time_tv, NULL); // NULL for timezone

    date = get_current_date();
    tk_time_of_day_t time = get_current_time_coarse();
    ESP_LOGI(LOG_TAG, "Changed time, current: %04d/%02d/%02d %02d:%02d:%02d", date.year, date.month, date.day, time.hour, time.minute, time.second);
}

void set_current_date(tk_date_t date) {
    tk_time_of_day_t time = get_current_time_precise();
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 
    t.tm_year = date.year - 1900;
    t.tm_mon = date.month - 1;
    t.tm_mday = date.day;
    t.tm_hour = time.hour;
    t.tm_min = time.minute;
    t.tm_sec = time.second;
    time_t timeSinceEpoch = mktime(&t);

    struct timeval new_time_tv;
    new_time_tv.tv_sec = timeSinceEpoch;
    new_time_tv.tv_usec = time.millisecond * 1000;
    settimeofday(&new_time_tv, NULL); // NULL for timezone

    date = get_current_date();
    time = get_current_time_coarse();
    ESP_LOGI(LOG_TAG, "Changed date, current: %04d/%02d/%02d %02d:%02d:%02d", date.year, date.month, date.day, time.hour, time.minute, time.second);
}

tk_time_of_day_t get_uptime() {
    int64_t uptime_us = esp_timer_get_time();
    int64_t uptime_ms = uptime_us / 1000;
    int64_t uptime_s = uptime_ms / 1000;
    int64_t uptime_m = uptime_s / 60;
    int64_t uptime_h = uptime_m / 60;

    return {
        .hour = (int) uptime_h,
        .minute = (int) uptime_m % 60,
        .second = (int) uptime_s % 60,
        .millisecond = (int) uptime_ms % 1000
    };
}

// Why not std::chrono? idk, legacy I guess

tk_time_of_day operator -(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    tk_time_of_day result = { 0 };

    int64_t a_ms = (a.hour * 60 * 60 + a.minute * 60 + a.second) * 1000 + a.millisecond;
    int64_t b_ms = (b.hour * 60 * 60 + b.minute * 60 + b.second) * 1000 + b.millisecond;

    int64_t diff_ms = a_ms - b_ms;

    if (diff_ms < 0) {
        // wrap around so that e.g 00:05 - 0h10min returns 23:55
        diff_ms += 24 * 60 * 60 * 1000;
    }

    result.hour = diff_ms / (60 * 60 * 1000);
    diff_ms %= (60 * 60 * 1000);
    result.minute = diff_ms / (60 * 1000);
    diff_ms %= (60 * 1000);
    result.second = diff_ms / 1000;
    result.millisecond = diff_ms % 1000;

    return result;
}


bool operator==(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    int64_t a_ms = (a.hour * 60 * 60 + a.minute * 60 + a.second) * 1000 + a.millisecond;
    int64_t b_ms = (b.hour * 60 * 60 + b.minute * 60 + b.second) * 1000 + b.millisecond;
    return a_ms == b_ms;
}

bool operator<(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    int64_t a_ms = (a.hour * 60 * 60 + a.minute * 60 + a.second) * 1000 + a.millisecond;
    int64_t b_ms = (b.hour * 60 * 60 + b.minute * 60 + b.second) * 1000 + b.millisecond;
    return a_ms < b_ms;
}

bool operator<=(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    return (a < b) || (a == b);
}

bool operator>(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    int64_t a_ms = (a.hour * 60 * 60 + a.minute * 60 + a.second) * 1000 + a.millisecond;
    int64_t b_ms = (b.hour * 60 * 60 + b.minute * 60 + b.second) * 1000 + b.millisecond;
    return a_ms > b_ms;
}

bool operator>=(const tk_time_of_day_t& a, const tk_time_of_day_t& b) {
    return (a > b) || (a == b);
}
