/**
 * @file otaWebUpdater.h
 * @author Martin Verges <martin@verges.cc>
 * @version 0.1
 * @date 2023-01-08
 * 
 * @copyright Copyright (c) 2022 by the author alone
 * 
 * License: CC BY-NC-SA 4.0
 */

#ifndef OTAWEBUPDATER_h
#define OTAWEBUPDATER_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

struct OtaWebVersion {
  String date;
  String version;
};

void otaTask(void* param);

class OtaWebUpdater {
  public:
    // Is a version check currently running
    bool otaIsRunning = false;

    // Is a new version available
    bool newReleaseAvailable = false;

    // Interval to run the loop()
    TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

    // Prefix for all API endpoints
    String apiPrefix = "/api/ota";

    // Initialize the OtaWebUpdater
    OtaWebUpdater();

    // Destruct this object
    virtual ~OtaWebUpdater();

    // Attach a webserver (if not done on initialization)
    void attachWebServer(AsyncWebServer * srv);

    // Starts a new otaTask
    bool startBackgroundTask();

    // Ends a running otaTask
    void stopBackgroundTask();

    // The loop function called from the background Task
    void loop();

    // Check if there is a new version available
    bool checkAvailableVersion();

    // Execute update
    void executeUpdate();

    // Install a new firmware version
    bool updateFile(String baseUrl, String filename);

    // Set a new baseUrl
    void setBaseUrl(String newUrl) { baseUrl = newUrl; };

    // Get a the baseUrl
    String getBaseUrl() { return baseUrl; };

    // Set a different check interval
    void setVersionCheckInterval(uint32_t minutes) {
      intervalVersionCheckMillis = minutes * 60 * 1000;
    }

    // Set OTA password
    void setOtaPassword(String newPass) { otaPassword = newPass; }

    // Set Firmware information
    void setFirmware(String fwDate, String fwRelease) {
      currentFwDate = fwDate;
      currentFwRelease = fwRelease;
    }

  private:
    // URL to load the data from
    // Files that needs to be located at this URL:
    //  - current-version.json       json with version information
    //  - boot_app0.bin              ESP32 boot code 
    //  - bootloader_dio_80m.bin     ESP32 boot code
    //  - partitions.bin             ESP32 flash partition layout
    //  - firmware.bin               This firmware file
    //  - littlefs.bin               The WebUI spiffs/littlefs
    String baseUrl = "http://s3.womolin.de/webinstaller/waterlevel-release";

    // The Webserver to register routes on
    AsyncWebServer * webServer;

    // Task handle for the background task
    TaskHandle_t otaCheckTask = NULL;

    // Time of last version check
    uint64_t lastVersionCheckMillis = 0;
    
    // Interval to check for new versions (should be hours!!)
    uint64_t intervalVersionCheckMillis = 12 * 60 * 60 * 1000; // 12 hours

    // Current running firmware compile date
    String currentFwDate = "";

    // Current running firmware release version
    String currentFwRelease = "";

    // Is the network ready?
    bool networkReady = false;

    // Inital check executed
    bool initialCheck = false;

    // Password to execute OTA upload
    String otaPassword = "";
};

#endif // OTAWEBUPDATER_h
