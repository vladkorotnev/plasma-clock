#include <device_config.h>
#include "../sensor.h"

#if HAS(SWITCHBOT_METER_INTEGRATION)
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>

typedef enum wosensor_dev_type : unsigned char {
    WO_DEVTYPE_HAND = 'H',
    WO_DEVTYPE_BUTTON = 'B',
    WO_DEVTYPE_LINK_UNPAIRED = 'L',
    WO_DEVTYPE_LINK_PAIRED = 'l',
    WO_DEVTYPE_LINKPLUS_UNPAIRED = 'P',
    WO_DEVTYPE_LINKPLUS_PAIRED = 'p',
    WO_DEVTYPE_FAN_UNPAIRED = 'F',
    WO_DEVTYPE_FAN_PAIRED = 'f',

    // WHY ARE THOSE REVERSED? 
    // I did not make this up:
    // https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/blob/latest/devicetypes/meter.md#switchBot-mode-(default):~:text=SwitchBot%20MeterTH%20(WoSensorTH)
    WO_DEVTYPE_SENSOR_TH_UNPAIRED = 't',
    WO_DEVTYPE_SENSOR_TH_PAIRED = 'T',
    
    WO_DEVTYPE_HUB_MINI_UNPAIRED = 'M',
    WO_DEVTYPE_HUB_MINI_PAIRED = 'm',
} wosensor_dev_type_t;

// NB: Endianness?
typedef struct __attribute__((packed)) wosensor_display_data {
    // Byte 0
    unsigned short temperature_decimal : 4;
    bool alarm_humidity_under : 1;
    bool alarm_humidity_over : 1;
    bool alarm_temperature_under : 1;
    bool alarm_temperature_over : 1;

    // Byte 1
    unsigned char temperature_celsius : 7;
    bool temperature_sign : 1; // why??

    // Byte 2
    unsigned short humidity_percent : 7;
    bool is_fahrenheit : 1;
} wosensor_display_data_t;

// NB: Endianness?
typedef struct __attribute__((packed)) wosensor_advertise_data {
    // Byte 0
    wosensor_dev_type_t device_type : 7;
    bool _reserved1 : 1;

    // Byte 1
    bool groupA : 1;
    bool groupB : 1;
    bool groupC : 1;
    bool groupD : 1;
    short _reserved2 : 4;

    // Byte 2
    unsigned short battery : 7;
    bool _reserved3 : 1;

    // Byte 3 onwards
    wosensor_display_data_t display;
} wosensor_advertise_data_t;

class SwitchbotMeterApi {
public:
    SwitchbotMeterApi(const char * macAddress);
    ~SwitchbotMeterApi();

    bool poll();
    bool get_data(wosensor_display_data_t *);

private:
    void prepare_client();
    SemaphoreHandle_t cacheSemaphore;

    wosensor_display_data_t last_data = { 0 };
    bool have_data = false;
    char desiredMac[20] = { 0 };
    bool polling = false;

    BLEClient * client;
};

#endif