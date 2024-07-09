#include "sensor/rssi.h"
#include <network/netmgr.h>

RssiSensor::RssiSensor() {
}

bool RssiSensor::poll(int* result) {
    if(NetworkManager::is_up()) {
        *result = NetworkManager::rssi();
    } else {
        *result = 1;
    }

    return true;
}