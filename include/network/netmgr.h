#include <WiFi.h>
#pragma once

/// @brief Manages the WiFi connectivity.
/// TODO: Refactor into simple C methods akin to other services
class NetworkManager {
public:
    /// @brief Initialize the network service and connect to the saved network, if it's available, otherwise start an ad-hoc network
    static void startup();
    /// @brief Whether the network service is initialized and connected or hosting an ad-hoc network 
    static bool is_up();

    /// @brief Current IP address string 
    static String current_ip();
    /// @brief Current network name string 
    static const char * network_name();

    /// @brief Connect to a specified network. If connection succeeds, the network credentials are saved into NVRAM.
    static void connect(const char * ssid, const char * pass);

private:
    static void wifi_event(WiFiEvent_t);
    static void save_current_network();
    static bool has_saved_credentials();
    /// @brief Starts the fallback Ad-Hoc network
    static void ap_fallback();
};