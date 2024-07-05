#include "views/signal_icon.h"
#include <service/prefs.h>
#include <network/netmgr.h>

#define RSSI_LOWEST_DBM -90

// 'sig_awful', 16x8px
const unsigned char icon_data_sig_awful[] = {
	0xfe, 0x00, 0x92, 0x00, 0x54, 0x00, 0x38, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00
};
// 'sig_excellent', 16x8px
const unsigned char icon_data_sig_excellent[] = {
	0xfe, 0x00, 0x92, 0x00, 0x54, 0x40, 0x38, 0x40, 0x11, 0x40, 0x11, 0x40, 0x15, 0x40, 0x15, 0x40
};
// 'sig_good', 16x8px
const unsigned char icon_data_sig_good[] = {
	0xfe, 0x00, 0x92, 0x00, 0x54, 0x00, 0x38, 0x00, 0x11, 0x00, 0x11, 0x00, 0x15, 0x00, 0x15, 0x00
};
// 'sig_outofrange', 16x8px
const unsigned char icon_data_sig_outofrange[] = {
	0xfe, 0x00, 0x92, 0x00, 0x54, 0x00, 0x38, 0x00, 0x10, 0x00, 0x15, 0x00, 0x12, 0x00, 0x15, 0x00
};
// 'sig_poor', 16x8px
const unsigned char icon_data_sig_poor[] = {
	0xfe, 0x00, 0x92, 0x00, 0x54, 0x00, 0x38, 0x00, 0x10, 0x00, 0x10, 0x00, 0x14, 0x00, 0x14, 0x00
};

SignalStrengthIcon::SignalStrengthIcon(SensorPool *s) {
    sensors = s;
    currentRssi = -127;
    isShowing = true;

    currentIcon  = {
        .width = 16, // TODO: support sprites less than a byte in width but more than 1 byte
        .height = 8,
        .data = icon_data_sig_outofrange
    };
}

void SignalStrengthIcon::step() {
    prefs_wifi_icon_disp_t dispRule = (prefs_wifi_icon_disp_t)prefs_get_int(PREFS_KEY_WIFI_ICON);
    if(dispRule == WIFI_ICON_DISP_NEVER) return;

    sensor_info_t * rssiInfo = sensors->get_info(VIRTSENSOR_ID_WIRELESS_RSSI);
    if(rssiInfo) {
        currentRssi = rssiInfo->last_result;

        switch(dispRule) {
            case WIFI_ICON_DISP_NEVER:
                isShowing = false;
                break;
            
            case WIFI_ICON_DISP_DISCONNECTED:
                isShowing = (currentRssi <= RSSI_LOWEST_DBM || currentRssi > 0);
                break;

            case WIFI_ICON_DISP_STARTLED:
                {
                    sensor_info_t * triggerInfo = sensors->get_info(VIRTSENSOR_ID_PMU_STARTLED);
                    if(triggerInfo) {
                        isShowing = triggerInfo->last_result || (currentRssi <= RSSI_LOWEST_DBM || currentRssi > 0);
                    } else {
                        isShowing = true;
                    }
                }
                break;

            case WIFI_ICON_DISP_ALWAYS:
                isShowing = true;
                break;
        }
    } else {
        isShowing = false;
    }

    if(isShowing) {
        if(currentRssi > 0) {
            // Disconnected
            currentIcon.data = icon_data_sig_outofrange;
        }
        else if(currentRssi >= -65) {
            currentIcon.data = icon_data_sig_excellent;
        }
        else if(currentRssi >= -70) {
            currentIcon.data = icon_data_sig_good;
        }
        else if(currentRssi >= -75) {
            currentIcon.data = icon_data_sig_poor;
        }
        else if(currentRssi >= -80) {
            currentIcon.data = icon_data_sig_awful;
        }
        else {
            currentIcon.data = icon_data_sig_outofrange;
        }
    }
}

void SignalStrengthIcon::render(FantaManipulator *fb) {
    if(!isShowing) return;

    fb->put_sprite(&currentIcon, fb->get_width() - 10, fb->get_height() - currentIcon.height);
}