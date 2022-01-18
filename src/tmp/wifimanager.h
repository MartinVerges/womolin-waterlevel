/**
 *
 * Wifi Manager
 *
 * (c) 2022 Martin Verges
 *
**/

/*
#ifndef WIFIMANAGER_h
#define WIFIMANAGER_h

#include <Arduino.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

#include <vector>

using std::vector;

class WIFIMANAGER {
  private:
    AsyncWebServer webServer = NULL;   // The Webserver to register routes on
    String apiPrefix = "/api/wifi";    // Prefix for all IP endpionts

    struct apCredentials_t {
      String apName;                   // Name of the AP SSID
      String apPass;                   // Password if required to the AP
    };

    vector<apCredentials_t> apList;    // List of known APs to connect to

    bool autoCreateAP = true;          // Create an AP for configuration if no other connection is available

    uint64_t lastWifiCheck = 0;        // Time of last Wifi health check
    uint32_t intervalWifiCheck = 1000; // Interval of the Wifi health checks

    // Register the API Routes to the webserver, called by attachWebServer()
    bool registerApiEndpoints();

  public:

    // Start the Wifi stack, try to connect to known APs
    bool begin();

    // Attach a webserver and register api routes
    void attachWebServer(AsyncWebServer &srv);

    // Detach from the Webserver
    void detachWebserver();

    // Add another AP to the list of known WIFIs
    bool addAp(String apName, String apPass);

    // Connect to AP with credentials
    bool connectTo(String apName, String apPass);

    // Run in the loop to maintain state
    void loop();

};


#endif
*/