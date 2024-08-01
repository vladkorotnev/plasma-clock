#include <network/otafvu.h>
#include <device_config.h>
#include <os_config.h>

#if HAS(OTAFVU)
#include <sound/melodies.h>
#include <fonts.h>
#include <service/power_management.h>
#include <state.h>
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

OTAFVUManager::OTAFVUManager(Console* c, NewSequencer*s) {
    ESP_LOGI(LOG_TAG, "Initializing");
    con = c;
    seq = s;

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
        pisosTASK_PRIORITY_OTAFVU,
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
    change_state(STATE_OTAFVU);

    con->set_active(true);
    con->clear();
    con->print("OTA FVU RECV\n");
    con->flush();
    con->set_cursor(false);
    con->set_font(&one_pixel_bar_font);
    con->write('|');

    // Keep display on when updating
    power_mgmt_pause();

    seq->play_sequence(tulula_fvu, SEQUENCER_NO_REPEAT);
}

void OTAFVUManager::shut_up_and_explode() {
    ESP_LOGI(LOG_TAG, "Shut up and explode!");
    con->set_font(&keyrus0816_font);
    con->clear();
    con->print("OTAFVU Done!");
    seq->play_sequence(oelutz_fvu, SEQUENCER_NO_REPEAT);
    seq->wait_end_play();
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
    static int lastPercent = 0;
    unsigned int percent =  (progress / (total / 100));
    ESP_LOGV(LOG_TAG, "Recv: %u%% [%u / %u]", percent, progress, total);
    while(lastPercent < percent) {
        con->write('|');
        lastPercent++;
    }
}
#endif