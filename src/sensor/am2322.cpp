#include <sensor/am2322.h>
#include <Arduino.h>

static char LOG_TAG[] = "AM2322";

Am2322HumiditySensor::Am2322HumiditySensor(AM232X* sensor) {
    hw = sensor;
}

Am2322TemperatureSensor::Am2322TemperatureSensor(AM232X* sensor) {
    hw = sensor;
}

bool Am2322HumiditySensor::initialize() {
    return hw->begin();
}

bool Am2322TemperatureSensor::initialize() {
    return hw->begin();
}

bool Am2322HumiditySensor::teardown() {
    return true;
}

bool Am2322TemperatureSensor::teardown() {
    return true;
}

bool Am2322HumiditySensor::poll(int* result) {
    hw->wakeUp();
    int rslt = hw->read();
    if(rslt == AM232X_OK || rslt == AM232X_READ_TOO_FAST) {
        float humidity = hw->getHumidity();
        int integerHumidity = humidity * 100.0;
        *result = integerHumidity;
        return true;
    } else {
        ESP_LOGE(LOG_TAG, "Error %i reading AM2322 sensor", rslt);
        return false;
    }
}

bool Am2322TemperatureSensor::poll(int* result) {
    hw->wakeUp();
    int rslt = hw->read();
    if(rslt == AM232X_OK || rslt == AM232X_READ_TOO_FAST) {
        float temperature = hw->getTemperature();
        int integerTemp = temperature * 100.0;
        *result = integerTemp;
        return true;
    } else {
        ESP_LOGE(LOG_TAG, "Error %i reading AM2322 sensor", rslt);
        return false;
    }
}