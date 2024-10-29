#include "app/httpfvu.h"
#include <service/httpfvu.h>
#include <fonts.h>
#include <service/localize.h>
#include <service/prefs.h>
#include <service/time.h>
#include <service/alarm.h>
#include <os_config.h>
#include <state.h>
#include <algorithm>

static char LOG_TAG[] = "HFVUAPP";
static TaskHandle_t hWorkTask = NULL;
static TaskHandle_t hCheckerTask = NULL;
static size_t total = 1;
static size_t done = 0;
static bool is_auto_entry = false;

#if HAS(HTTPFVU)
static void fvuCheckerTask(void*) {
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(std::max(prefs_get_int(PREFS_KEY_FVU_AUTO_CHECK_INTERVAL_MINUTES), 1) * 60000));
        if(prefs_get_bool(PREFS_KEY_FVU_AUTO_CHECK)) {
            ESP_LOGI(LOG_TAG, "Checking for firmware updates");
            httpfvu_new_version_info_t ver = get_remote_version_info();
            if(prefs_get_bool(PREFS_KEY_FVU_AUTO_INSTALL) && ver.fs_current != nullptr && ver.fs_new != nullptr && strcmp(ver.fs_current, ver.fs_new) != 0) {
                // New version auto install
                tk_time_of_day_t now = get_current_time_coarse();
                if(
                    // If hourly chime is enabled, use it's schedule as the time to allow updating
                    (prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON) && now.hour >= prefs_get_int(PREFS_KEY_HOURLY_CHIME_START_HOUR) && now.hour <= prefs_get_int(PREFS_KEY_HOURLY_CHIME_STOP_HOUR)) ||
                    // Otherwise update from 10am til 10pm
                    (!prefs_get_bool(PREFS_KEY_HOURLY_CHIME_ON) && now.hour >= 10 && now.hour <= 22)
                ) {
                    // Make sure there is no upcoming alarm -- we don't want to disrupt it with a potentially fatal update failure
                    if(get_upcoming_alarm() == nullptr) {
                        while(get_state() != STATE_IDLE) {
                            vTaskDelay(pdMS_TO_TICKS(10000));
                        }
                        is_auto_entry = true;
                        push_state(STATE_HTTPFVU);
                    }
                }
            }
        }
    }
}
#endif

static void fvuWorkTaskShim(void* there) {
    HttpFvuApp* that = static_cast<HttpFvuApp*>(there);
    that->work_task();
}

HttpFvuApp::HttpFvuApp(NewSequencer *s) {
    wants_clear_surface = true;
    
    progBar = new Swoopie();
    label = new StringScroll(&keyrus0808_font);
    label->start_at_visible = true;
    label->holdoff = 100;
    sequencer = s;
    _oldState = FVUAPP_INIT;
    appState = FVUAPP_INIT;

#if HAS(HTTPFVU)
    if(hCheckerTask == NULL) {
        if(!xTaskCreate(
            &fvuCheckerTask,
            "FVUCHK",
            8000,
            nullptr,
            pisosTASK_PRIORITY_FVUCHECKER,
            &hCheckerTask
        )) {
            ESP_LOGE(LOG_TAG, "Failed to start checker task");
        }
    }

    add_composable(label);
    add_subrenderable(progBar);
#endif
}

void HttpFvuApp::prepare() {
    Composite::prepare();
#if HAS(HTTPFVU)
    _oldState = FVUAPP_INIT;
    progBar->value = -1;
    delayCounter = 0;
    if(is_auto_entry) {
        appState = FVUAPP_INSTALLING_APP;
    } else {
        appState = FVUAPP_CHECKING;
    }

    if(hCheckerTask != NULL) vTaskSuspend(hCheckerTask);

    if(hWorkTask == NULL) {
        if(!xTaskCreate(
            &fvuWorkTaskShim,
            "FVUWRK",
            8000,
            this,
            pisosTASK_PRIORITY_FVUPROCESS,
            &hWorkTask
        )) {
            ESP_LOGE(LOG_TAG, "Failed to start work task");
            appState = FVUAPP_FAILED;
        }
    }
#endif
}

void HttpFvuApp::cleanup() {
    Composite::cleanup();
    if(hCheckerTask != NULL && appState != FVUAPP_SUCCESS) vTaskResume(hCheckerTask);
}

void HttpFvuApp::step() {
    Composite::step();

    if(_oldState != appState) {
        switch(appState) {
            case FVUAPP_CHECKING: 
                label->set_string(localized_string("Checking for update"));
                break;
            case FVUAPP_INSTALLING_APP:
                if(is_auto_entry) {
                    sequencer->play_sequence(&tulula_fvu);
                }
                is_auto_entry = false;
                label->set_string(localized_string("Downloading firmware"));
                break;
            case FVUAPP_INSTALLING_FS:
                label->set_string(localized_string("Downloading filesystem"));
                break;
            case FVUAPP_NO_UPDATES:
                label->set_string(localized_string("No new version"));
                break;
            case FVUAPP_FAILED:
                sequencer->play_sequence(&tulula_fvu);
                label->set_string(localized_string("Update failed"));
                break;
            case FVUAPP_SUCCESS:
                sequencer->play_sequence(&oelutz_fvu);
                label->set_string(localized_string("Update successful"));
                break;
        }
        _oldState = appState;
    }

    if(appState == FVUAPP_INSTALLING_FS || appState == FVUAPP_INSTALLING_APP) {
        progBar->maximum = total;
        progBar->value = done;
    }

#if HAS(HTTPFVU)
    if(appState == FVUAPP_SUCCESS || appState == FVUAPP_NO_UPDATES || appState == FVUAPP_FAILED) {
        delayCounter++;
    }

    if(appState == FVUAPP_FAILED || appState == FVUAPP_NO_UPDATES) {
        if(hid_test_key_state(KEY_LEFT) || delayCounter == 300) {
            pop_state(STATE_HTTPFVU, TRANSITION_SLIDE_HORIZONTAL_RIGHT);
        }
    }
    else if(appState == FVUAPP_SUCCESS) {
        if(hid_test_key_any() || delayCounter == 300) {
            change_state(STATE_RESTART, TRANSITION_NONE);
        }
    }
#endif
}

void HttpFvuApp::work_task() {
    vTaskDelay(pdMS_TO_TICKS(1000));
    while(appState != FVUAPP_FAILED && appState != FVUAPP_SUCCESS && appState != FVUAPP_NO_UPDATES) {
        switch(appState) {
            case FVUAPP_CHECKING:
                {
                    ESP_LOGI(LOG_TAG, "Checking for firmware updates");
                    httpfvu_new_version_info_t ver = get_remote_version_info();
                    if(ver.fs_current != nullptr && ver.fs_new != nullptr && strcmp(ver.fs_current, ver.fs_new) != 0) {
                        appState = FVUAPP_INSTALLING_APP;
                    }
                    else {
                        appState = FVUAPP_NO_UPDATES;
                    }
                }
                break;
            case FVUAPP_INSTALLING_APP:
                ESP_LOGI(LOG_TAG, "Installing APP");
                download_app([this](bool failed, bool complete, size_t _done, size_t _total) {
                    if(failed) {
                        this->appState = FVUAPP_FAILED;
                    }
                    else if(complete) {
                        this->appState = FVUAPP_INSTALLING_FS;
                    }
                    else {
                        total = _total;
                        done = _done;
                    }
                });
                break;

            case FVUAPP_INSTALLING_FS:
                ESP_LOGI(LOG_TAG, "Installing FS");
                download_fs([this](bool failed, bool complete, size_t _done, size_t _total) {
                    if(failed) {
                        this->appState = FVUAPP_FAILED;
                    }
                    else if(complete) {
                        this->appState = FVUAPP_SUCCESS;
                    }
                    else {
                        total = _total;
                        done = _done;
                    }
                });
                break;
            default:
                break;
        }
        taskYIELD();
    }
    hWorkTask = NULL;
    vTaskDelete(hWorkTask);
}