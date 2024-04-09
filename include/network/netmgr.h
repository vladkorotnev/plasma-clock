#include <WiFi.h>
#pragma once

class NetworkManager {
public:
    static void startup();
    static bool is_up();

    static String current_ip();
    static const char * network_name();

    static void connect(const char *, const char *);

private:
    static void wifi_event(WiFiEvent_t);
    static void save_current_network();
    static bool has_saved_credentials();
    static void ap_fallback();
};