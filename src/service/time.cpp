#include <service/time.h>
#include <service/prefs.h>
#include <esp_sntp.h>
#include <esp32-hal-log.h>

static char LOG_TAG[] = "TIME";
static char * ntp_server = nullptr;

void timekeeping_begin() {
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

    String tz = prefs_get_string(PREFS_KEY_TIMEZONE, String(TK_TIMEZONE));
    ESP_LOGI(LOG_TAG, "Timezone: %s", tz.c_str());
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