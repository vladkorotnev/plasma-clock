#include <network/netmgr.h>
#include <service/prefs.h>
#include <utils.h>

static char LOG_TAG[] = "NetM";

static bool initial_connection_succeeded;
static bool initial_connection_ongoing;
static bool reconnecting;
static bool has_ip;

static String ssid;
static String pass;

bool NetworkManager::has_saved_credentials() {
    return prefs_get_string(PREFS_KEY_WIFI_SSID).length() > 0;
}

void NetworkManager::startup() {
    initial_connection_succeeded = false;

    WiFi.persistent(false);
    WiFi.onEvent(wifi_event);
    
    if(WiFi.status() != WL_CONNECTED) {
        if(has_saved_credentials()) {
            ESP_LOGI(LOG_TAG, "Attempt initial connection to saved network...");

            ssid = prefs_get_string(PREFS_KEY_WIFI_SSID);
            pass = prefs_get_string(PREFS_KEY_WIFI_PASS);

            initial_connection_ongoing = true;
            WiFi.mode(WIFI_MODE_STA);
            WiFi.begin(ssid, pass);
        } else {
            ESP_LOGI(LOG_TAG, "No saved network, use AP fallback");
            ap_fallback();
        }
    }
}

void NetworkManager::wifi_event(WiFiEvent_t ev) {
    switch(ev) {
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(LOG_TAG, "STA_DISCONNECTED");
            if(!initial_connection_succeeded) {
                ESP_LOGI(LOG_TAG, "Initial connection failed, fall back to AP mode");
                ap_fallback();
            } else if (reconnecting) {
                // expect disconnect before next connection is made
                reconnecting = false;
            } else {
                ESP_LOGI(LOG_TAG, "Connection lost, reconnecting to %s in a few seconds...", ssid.c_str());
                vTaskDelay(pdMS_TO_TICKS(5000));
                WiFi.mode(WIFI_MODE_STA);
                WiFi.begin(ssid, pass);
            }
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(LOG_TAG, "Connection succeeded. SSID:'%s', IP: %s", network_name(), current_ip().c_str());

            has_ip = true;
            if(initial_connection_ongoing) {
                initial_connection_succeeded = true;
            } else {
                save_current_network();
            }
            ESP_LOGI(LOG_TAG, "RSSI: %i dB", rssi());
            break;
        default:
            ESP_LOGI(LOG_TAG, "Unhandled event %i", ev);
            break;
    }
}

void NetworkManager::save_current_network() {
    prefs_set_string(PREFS_KEY_WIFI_SSID, ssid);
    prefs_set_string(PREFS_KEY_WIFI_PASS, pass);
}

void NetworkManager::connect(const char * name, const char * pw) {
    ssid = String(name);
    pass = String(pw);
    reconnecting = true;
    has_ip = false;
    WiFi.disconnect(false, true);
    WiFi.mode(WIFI_MODE_STA);
    wl_status_t rslt = WiFi.begin(ssid, pass);
    if(rslt != WL_CONNECTED) {
        ESP_LOGI(LOG_TAG, "WiFi connection error (%i): fallback to SoftAP");
        ap_fallback();
    }
}

const char * NetworkManager::network_name() {
    return ssid.c_str();
}

bool NetworkManager::is_up() {
    return (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED && has_ip)
        || is_softAP();
}

bool NetworkManager::is_softAP() {
    return (WiFi.getMode() == WIFI_AP && (WiFi.getStatusBits() & AP_STARTED_BIT != 0));
}

String NetworkManager::current_ip() {
    if(WiFi.getMode() == WIFI_STA)
        return WiFi.localIP().toString();
    else if(WiFi.getMode() == WIFI_AP)
        return WiFi.softAPIP().toString();
    else
        return "0.0.0.0";
}

void NetworkManager::ap_fallback() {
    WiFi.disconnect(false, true);
    WiFi.mode(WIFI_MODE_AP);
    ssid = String("pisos_" + getChipId());
    ESP_LOGI(LOG_TAG, "Starting AP");
    WiFi.softAP(ssid);
}

int NetworkManager::rssi() {
    return is_softAP() ? 1 : WiFi.RSSI();
}