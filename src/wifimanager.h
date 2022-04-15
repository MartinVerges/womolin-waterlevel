/**
 *
 * Wifi Manager
 *
 * (c) 2022 Martin Verges
 *
**/

#ifndef WIFIMANAGER_h
#define WIFIMANAGER_h

#define WIFIMANAGER_MAX_APS 4

#include <Arduino.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

class WIFIMANAGER {
  private:
    AsyncWebServer * webServer;        // The Webserver to register routes on
    String apiPrefix = "/api/wifi";    // Prefix for all IP endpionts

    Preferences preferences;           // Used to store AP credentials to NVS
    char * NVS;                        // Name used for NVS preferences

    struct apCredentials_t {
      String apName;                   // Name of the AP SSID
      String apPass;                   // Password if required to the AP
    };
    apCredentials_t apList[WIFIMANAGER_MAX_APS];  // Stored AP list

    bool autoCreateAP = true;          // Create an AP for configuration if no other connection is available

    uint64_t lastWifiCheck = 0;        // Time of last Wifi health check
    uint32_t intervalWifiCheck = 1000; // Interval of the Wifi health checks
    uint64_t startApTime = 0;          // Time when the AP was started
    uint32_t timeoutApMillis = 300000; // Timeout of an AP when no client is connected, if timeout reached rescan, tryconnect or createAP

  public:
    // We let the loop run as as Task
    TaskHandle_t WifiCheckTask;

    WIFIMANAGER(const char * ns = "wifimanager");
    virtual ~WIFIMANAGER();

    // Call to run the Task 
    void start();

    // Attach a webserver and register api routes
    void attachWebServer(AsyncWebServer * srv);

    // Add another AP to the list of known WIFIs
    bool addAp(String apName, String apPass);

    // Try each known SSID and connect until none is left or one is connected.
    bool tryConnect();

    // Start a SoftAP, called if no wifi can be connected
    bool runAP(String apName = "");

    // Run in the loop to maintain state
    void loop();

    // Write AP Settings into persistent storage. Called on each addAP;
    bool writeToNVS();

    // Load AP Settings from NVS it known apList
    bool loadFromNVS();
};


#endif
