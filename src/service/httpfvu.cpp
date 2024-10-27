#include "service/httpfvu.h"
#include <service/prefs.h>
#include <device_config.h>
#include <string.h>
#include <esp_ota_ops.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#if !HAS(HTTPFVU)
#define NO_GLOBAL_UPDATE
#endif
#include <Update.h>

static char LOG_TAG[] = "HTTPFVU";

#define APP_VERUP_EXTENSION ".avu"
#define FS_VERUP_REMOTE_FILENAME "fs.fvu"
#define FS_VER_REMOTE_FILENAME "fs_ver.txt"

static httpfvu_new_version_info_t cur_version = { 0 };

void load_fs_ver() {
    if(cur_version.fs_current != nullptr) {
        free((char*)cur_version.fs_current);
        cur_version.fs_current = nullptr;
    }

    FILE * f = fopen(FS_MOUNTPOINT "/FS_VER", "r");
    if(f) {
        fseek(f, 0, SEEK_END);
        size_t sz = ftell(f);
        fseek(f, 0, SEEK_SET);

        char * fs_ver_str = (char*) malloc(sz + 1);
        fread(fs_ver_str, 1, sz, f);
        fs_ver_str[sz] = 0;

        // Trim newlines at the end
        for(int i = strlen(fs_ver_str) - 1; i >= 0 && (fs_ver_str[i] == '\n' || fs_ver_str[i] == '\r'); i--) {
            fs_ver_str[i] = 0;
        }

        fclose(f);

        cur_version.fs_current = fs_ver_str;
    } else {
        cur_version.fs_current = "ERROR";
    }
    ESP_LOGI(LOG_TAG, "Get Local FS version: %s", cur_version.fs_current);
}

void load_fs_ver_if_needed() {
    if(cur_version.fs_current != nullptr) return;

    load_fs_ver();
}

httpfvu_new_version_info_t get_current_version_info() {
    load_fs_ver_if_needed();
    return cur_version;
}

#if HAS(HTTPFVU)
void load_remote_fs_ver() {
    String url = prefs_get_string(PREFS_KEY_FVU_SERVER, FVU_SERVER_URL) + "/" + FS_VER_REMOTE_FILENAME;
    ESP_LOGI(LOG_TAG, "Load FS ver from %s", url.c_str());
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    int response = http.GET();
    if(response == HTTP_CODE_OK) {
        String ver = http.getString();
        ver.trim();
        size_t len = ver.length();
        char * dst;
        if(cur_version.fs_new != nullptr) {
            dst = (char*) realloc((void*)cur_version.fs_new, len + 1);
        } else {
            dst = (char*) malloc(len + 1);
        }
        cur_version.fs_new = dst;
        strncpy(dst, ver.c_str(), len);
        dst[len] = 0;
        for(int i = strlen(dst) - 1; i >= 0 && (dst[i] == '\n' || dst[i] == '\r'); i--) {
            dst[i] = 0;
        }
        ESP_LOGI(LOG_TAG, "Remote FS version: %s", dst);
    } else {
        ESP_LOGE(LOG_TAG, "Error when checking for FS version: %i", response);
    }
    client.stop();
}

void download_fs(httpfvu_progress_cb_t progress_callback) {
    Update.onProgress([progress_callback](size_t done, size_t total) {
        if(!Update.hasError()) {
            if(done == total || (done % 10240) == 0) {
                ESP_LOGI(LOG_TAG, "FS Download: %i of %i bytes", done, total);
            }
            progress_callback(false, false, done, total);
        }
    });

    String url = prefs_get_string(PREFS_KEY_FVU_SERVER, FVU_SERVER_URL) + ("/" FS_VERUP_REMOTE_FILENAME);
    ESP_LOGI(LOG_TAG, "Load FS from %s", url.c_str());
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    int response = http.GET();
    if(response == HTTP_CODE_OK) {
        LittleFS.end();
        int len = http.getSize();
        if(len == -1) len = UPDATE_SIZE_UNKNOWN;
        if(!Update.begin(len, U_SPIFFS)) {
            ESP_LOGE(LOG_TAG, "FS: Update prerequisite check failed");
            LittleFS.begin(true, FS_MOUNTPOINT);
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        size_t written = Update.writeStream(http.getStream());
        ESP_LOGI(LOG_TAG, "FS: Written %i bytes", written);
        if(Update.hasError()) {
            ESP_LOGE(LOG_TAG, "Update error: (%i) %s", Update.getError(), Update.errorString());
            LittleFS.begin(true, FS_MOUNTPOINT);
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        if(Update.end(true)) {
            ESP_LOGI(LOG_TAG, "Completed FS update");
            LittleFS.begin(true, FS_MOUNTPOINT);
        }
        if(Update.hasError()) {
            ESP_LOGE(LOG_TAG, "FS Update error: (%i) %s", Update.getError(), Update.errorString());
            LittleFS.begin(true, FS_MOUNTPOINT);
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        progress_callback(false, true, 0, 0);
        client.stop();
    } else {
        ESP_LOGE(LOG_TAG, "FS: Error when getting new file: %i", response);
        progress_callback(true, false, 0, 0);
    }
    client.stop();
}


void download_app(httpfvu_progress_cb_t progress_callback) {
    Update.onProgress([progress_callback](size_t done, size_t total) {
        if(!Update.hasError()) {
            if(done == total || (done % 10240) == 0) {
                ESP_LOGI(LOG_TAG, "APP Download: %i of %i bytes", done, total);
            }
            progress_callback(false, false, done, total);
        }
    });

    String url = prefs_get_string(PREFS_KEY_FVU_SERVER, FVU_SERVER_URL) + ("/" FVU_FLAVOR APP_VERUP_EXTENSION);
    ESP_LOGI(LOG_TAG, "Load APP from %s", url.c_str());
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    int response = http.GET();
    if(response == HTTP_CODE_OK) {
        int len = http.getSize();
        if(len == -1) len = UPDATE_SIZE_UNKNOWN;
        if(!Update.begin(len, U_FLASH)) {
            ESP_LOGE(LOG_TAG, "APP: Update prerequisite check failed");
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        size_t written = Update.writeStream(http.getStream());
        ESP_LOGI(LOG_TAG, "APP: Written %i bytes", written);
        if(Update.hasError()) {
            ESP_LOGE(LOG_TAG, "Update error: (%i) %s", Update.getError(), Update.errorString());
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        if(Update.end(true)) {
            ESP_LOGI(LOG_TAG, "Completed APP update");
        }
        if(Update.hasError()) {
            ESP_LOGE(LOG_TAG, "APP Update error: (%i) %s", Update.getError(), Update.errorString());
            progress_callback(true, false, 0, 0);
            client.stop();
            return;
        }
        progress_callback(false, true, 0, 0);
        client.stop();
    } else {
        ESP_LOGE(LOG_TAG, "APP: Error when getting new file: %i", response);
        progress_callback(true, false, 0, 0); 
    }
    client.stop();
}

#else
void download_fs(httpfvu_progress_cb_t progress_callback) {
    progress_callback(true, false, 0, 0);
}
void download_fs(httpfvu_progress_cb_t progress_callback) {
    progress_callback(true, false, 0, 0);
}
void load_remote_fs_ver() {}
#endif

httpfvu_new_version_info_t get_remote_version_info() {
    load_remote_fs_ver();
    return get_current_version_info();
}
