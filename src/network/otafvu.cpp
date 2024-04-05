#include <network/otafvu.h>
#include <service/power_management.h>
#include <utils.h>
#include <fonts.h>

static char LOG_TAG[] = "OTAFVU";

extern "C" void OtaFvuTaskFunction( void * pvParameter );

void OtaFvuTaskFunction( void * pvParameter )
{
    ESP_LOGV(LOG_TAG, "Running task");
    OTAFVUManager * that = static_cast<OTAFVUManager*> ( pvParameter );

    while(1) {
        that->task();
    }
}

OTAFVUManager::OTAFVUManager(Console* c) {
    ESP_LOGI(LOG_TAG, "Initializing");
    con = c;

    ArduinoOTA.setHostname((String("plasma-") + getChipId()).c_str());
#ifndef OTAFVU_PORT
#define OTAFVU_PORT 3232
#endif
    ArduinoOTA.setPort(OTAFVU_PORT);
#ifdef OTAFVU_PASSWORD_HASH
    ArduinoOTA.setPasswordHash(OTAFVU_PASSWORD_HASH);
#endif
    ArduinoOTA.setRebootOnSuccess(false);


    ArduinoOTA.onStart([this] { this->get_ready(); });
    ArduinoOTA.onProgress([this] (unsigned int got, unsigned int of) { this->on_progress(got, of); });
    ArduinoOTA.onError([this] (ota_error_t error) { this->on_error(error); });
    ArduinoOTA.onEnd([this] { this->shut_up_and_explode(); });

    ArduinoOTA.begin();

    ESP_LOGV(LOG_TAG, "Creating task");
    if(xTaskCreate(
        OtaFvuTaskFunction,
        "OTAFVU",
        4096,
        this,
        10,
        &hTask
    ) != pdPASS) {
        ESP_LOGE(LOG_TAG, "Task creation failed!");
    }
}

void OTAFVUManager::task() {
    ArduinoOTA.handle();
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void OTAFVUManager::get_ready() {
    ESP_LOGI(LOG_TAG, "Get ready");
    con->set_active(true);
    con->print("Start OTAFVU");

    // Keep display on when updating
    power_mgmt_pause();
}

void OTAFVUManager::shut_up_and_explode() {
    ESP_LOGI(LOG_TAG, "Shut up and explode!");
    con->print("OTAFVU Done!");
    ESP.restart();
}

void OTAFVUManager::on_error(ota_error_t error) {
    ESP_LOGE(LOG_TAG, "OTA error %i!!", error);
    con->clear();
    switch(error) {
        case OTA_AUTH_ERROR:
            con->print("Auth err!");
            break;
        case OTA_BEGIN_ERROR:
            con->print("Begin err!");
            break;
        case OTA_CONNECT_ERROR:
            con->print("Conn err!");
            break;
        case OTA_RECEIVE_ERROR:
            con->print("Recv err!");
            break;
        case OTA_END_ERROR:
            con->print("End error!");
            break;
    }
}

void OTAFVUManager::on_progress(unsigned int progress, unsigned int total) {
    unsigned int percent =  (progress / (total / 100));
    ESP_LOGV(LOG_TAG, "Recv: %u%% [%u / %u]", percent, progress, total);
    con->clear();
    con->print("Recv: %u%%", percent);
}