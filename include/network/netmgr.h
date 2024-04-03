#include <WiFi.h>
#include <Preferences.h>
#pragma once

class NetworkManager {
public:
    static constexpr char * PREFS_DOMAIN = "clockos-wifi";
    static constexpr char * PREFS_KEY_SSID = "ssid";
    static constexpr char * PREFS_KEY_PASS = "pass";

    static void startup();
    static bool is_up();

    static const char * current_ip();
    static const char * network_name();

    static void connect(const char *, const char *);

private:
    static void wifi_event(WiFiEvent_t);
    static void save_current_network();
    static bool has_saved_credentials();
    static void ap_fallback();
};