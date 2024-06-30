#include <device_config.h>
#include "../sensor.h"
#include "api.h"
#include <esp32-hal-log.h>

#if HAS(BLUETOOTH_LE) && HAS(SWITCHBOT_METER_INTEGRATION)

class SwitchbotMeterHumidity : public PoolableSensor {
public:
    SwitchbotMeterHumidity(SwitchbotMeterApi * a) {
        api = a;
    }

    bool initialize() {
        return api->poll();
    }

    bool poll(int* dst) {
        api->poll();
        wosensor_display_data_t tmp = { 0 };
        bool rslt = api->get_data(&tmp);
        if(rslt) {
            *dst = tmp.humidity_percent * 100;
            ESP_LOGI("RHum", "New value: %i", tmp.humidity_percent * 100);
        }
        return rslt;
    }

private:
    SwitchbotMeterApi * api;
};

class SwitchbotMeterTemperature : public PoolableSensor {
public:
    SwitchbotMeterTemperature(SwitchbotMeterApi * a) {
        api = a;
    }

    bool initialize() {
        return api->poll();
    }

    bool poll(int* dst) {
        api->poll();
        wosensor_display_data_t tmp = { 0 };
        bool rslt = api->get_data(&tmp);
        if(rslt) {
            *dst = (tmp.temperature_sign ? 1 : -1) * (tmp.temperature_celsius * 100 + tmp.temperature_decimal * 10);
            ESP_LOGI("RTemp", "New value: %i", tmp.humidity_percent * 100);
        }
        return rslt;
    }

private:
    SwitchbotMeterApi * api;
};

#endif