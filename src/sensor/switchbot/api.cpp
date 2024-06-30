#include <sensor/switchbot/api.h>
#include <esp32-hal-log.h>
#include <cstring>

#if HAS(SWITCHBOT_METER_INTEGRATION)

static char LOG_TAG[] = "WOMeter";
const BLEUUID serviceId = BLEUUID("CBA20D00-224D-11E6-9FB8-0002A5D5C51B");
const BLEUUID txCharaId = BLEUUID("cba20002-224d-11e6-9fb8-0002a5d5c51b");
const BLEUUID rxCharaId = BLEUUID("cba20003-224d-11e6-9fb8-0002a5d5c51b");

SwitchbotMeterApi::SwitchbotMeterApi(const char * mac) {
    polling = false;
    client = nullptr;

    if(!BLEDevice::getInitialized()) {
        BLEDevice::init("");
    }

    if(mac != nullptr && mac[0] != 0x0) {
        strncpy(desiredMac, mac, sizeof(desiredMac) - 1);
    }

    cacheSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(cacheSemaphore);
}

SwitchbotMeterApi::~SwitchbotMeterApi() {
    if(client != nullptr) {
        if(client->isConnected()) client->disconnect();
        client = nullptr;
    }
}

// void SwitchbotMeterApi::onResult(NimBLEAdvertisedDevice* dev) {
//     if(dev->haveManufacturerData()) {
//         for(int j = 0; j < dev->getManufacturerDataCount(); j++) {
//             std::string mfgData = dev->getManufacturerData(j);
//             const char * mfgDataRaw = mfgData.c_str();
//             if(mfgDataRaw[0] == 0x69 && mfgDataRaw[1] == 0x09) {
//                 ESP_LOGI(LOG_TAG, "Correct Mfg Data found: %s", dev->getAddress().toString().c_str());

//                 BLEUUID meterUUID = (uint16_t)0xFD3D;
//                 std::string data = dev->getServiceData(meterUUID);
//                 if(data.length() > 0) {
//                     const char * rawData = data.c_str();
//             const wosensor_advertise_data_t * advData = (const wosensor_advertise_data_t*) rawData;
//             ESP_LOGI(LOG_TAG, "DevType = %i", advData->device_type);
//             ESP_LOGI(LOG_TAG, "Humid = %i, temp = %i.%i, F = %i, batt = %i", advData->display.humidity_percent, advData->display.temperature_celsius, advData->display.temperature_decimal, advData->display.is_fahrenheit, advData->battery);
//                 }
//             }
//         }
//     }
// }

void SwitchbotMeterApi::prepare_client() {
    if(desiredMac == nullptr) return;

    if(client == nullptr) {
         BLEAddress goalAddress = BLEAddress(desiredMac, 1);
        BLEClient *c = BLEDevice::createClient(goalAddress);
        if(c != nullptr) {
            client = c;
        } else {
            ESP_LOGE(LOG_TAG, "Failed to create BLE client");
            return;
        }
    }

    if(client->connect(true)) {
        BLERemoteService *svc = client->getService(serviceId);
        if(svc != nullptr) {
            BLERemoteCharacteristic * rxChara = svc->getCharacteristic(rxCharaId);
            
            if(rxChara != nullptr) {
                rxChara->subscribe(true, [this] (NimBLERemoteCharacteristic* pBLERemoteCharacteristic,
                                uint8_t* pData, size_t length, bool isNotify) { 
                    ESP_LOGV(LOG_TAG, "Rx len = %i", length);
                    if(pData[0] == 1) {
                        const wosensor_display_data_t * dsp = (const wosensor_display_data_t *) &pData[1];
                        ESP_LOGI(LOG_TAG, "Humid = %i, temp = %i.%i, F = %i", dsp->humidity_percent,  dsp->temperature_celsius,  dsp->temperature_decimal, dsp->is_fahrenheit);
                        xSemaphoreTake(cacheSemaphore, portMAX_DELAY);
                        memcpy(&this->last_data, dsp, sizeof(wosensor_display_data_t));
                        this->have_data = true;
                        xSemaphoreGive(cacheSemaphore);
                    }
                    this->polling = false;
                    client->disconnect();
                });
            }
        }
    }
}

bool SwitchbotMeterApi::poll() {
    if(polling) {
        ESP_LOGW(LOG_TAG, "Already polling, doing nothing.");
        return true;
    }

    prepare_client();
  
    if(client != nullptr) {
        BLERemoteService *svc = client->getService(serviceId);
        if(svc != nullptr) {
            BLERemoteCharacteristic * txChara = svc->getCharacteristic(txCharaId);
            if(txChara != nullptr) {
                txChara->writeValue("\x57\x0F\x31");
                polling = true;
                return true;
            }
        }
    }

    return false;
}

bool SwitchbotMeterApi::get_data(wosensor_display_data_t * dst) {
    bool rslt = false;
    xSemaphoreTake(cacheSemaphore, portMAX_DELAY);

    if(have_data) {
        memcpy(dst, &last_data, sizeof(wosensor_display_data_t));
        rslt = true;
    }

    xSemaphoreGive(cacheSemaphore);
    return rslt;
}

#endif