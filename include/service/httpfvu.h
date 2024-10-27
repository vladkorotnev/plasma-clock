#pragma once
#include <functional>

// Someday TODO: also check APP version and allow independent updating. For now the Arduino-ESP32 blocks us from properly using the ESP APP header.
// That might also need a rewrite from the arduino style OTA APIs to the proper ESP-IDF ones.
typedef struct httpfvu_new_version_info {
    const char * fs_current;
    const char * fs_new;
} httpfvu_new_version_info_t;

typedef std::function<void(bool failed, bool completed, size_t done, size_t total)> httpfvu_progress_cb_t;

/// @brief Gets the current version info. Might contain remote version info too, but not guaranteed to be fresh.
httpfvu_new_version_info_t get_current_version_info();
/// @brief Check the version info on the remote server.
httpfvu_new_version_info_t get_remote_version_info();
/// @brief Synchronously update the filesystem image. 
void download_fs(httpfvu_progress_cb_t progress_callback = [](bool, bool, size_t, size_t){});
/// @brief Synchronously update the application image. 
void download_app(httpfvu_progress_cb_t progress_callback = [](bool, bool, size_t, size_t){});

