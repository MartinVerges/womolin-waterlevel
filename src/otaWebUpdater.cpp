/**
 * Wifi Manager
 * (c) 2022 Martin Verges
 *
 * Licensed under CC BY-NC-SA 4.0
 * (Attribution-NonCommercial-ShareAlike 4.0 International)
**/

#include "otaWebUpdater.h"

#include <AsyncJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <new>          // ::operator new[]

/**
 * @brief Construct a new OtaWebUpdater::OtaWebUpdater object
 */
OtaWebUpdater::OtaWebUpdater() {
  Serial.println(F("[OTAWEBUPDATER] Created, registering WiFi events"));

  if (WiFi.isConnected()) networkReady = true;

  auto eventHandlerUp = [&](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println(F("[OTAWEBUPDATER][WIFI] onEvent() Network connected"));
    networkReady = true;
  };
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_WIFI_STA_GOT_IP6);
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_ETH_GOT_IP);
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_ETH_GOT_IP6);

  auto eventHandlerDown = [&](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println(F("[OTAWEBUPDATER][WIFI] onEvent() Network disconnected"));
    networkReady = false;
  };
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(eventHandlerUp, ARDUINO_EVENT_ETH_DISCONNECTED);
}

/**
 * @brief Destroy the OtaWebUpdater::OtaWebUpdater object
 * @details will stop the background task as well but not cleanup the AsyncWebserver
 */
OtaWebUpdater::~OtaWebUpdater() {
  stopBackgroundTask();
  // FIXME: get rid of the registered Webserver AsyncCallbackWebHandlers
}

/**
 * @brief Attach to a webserver and register the API routes
 */
void OtaWebUpdater::attachWebServer(AsyncWebServer * srv) {
  webServer = srv; // store it in the class for later use

  webServer->on((apiPrefix + "/upload").c_str(), HTTP_POST,
    [&](AsyncWebServerRequest *request) { },
    [&](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {

    if (otaPassword.length()) {
      if(!request->authenticate("ota", otaPassword.c_str())) {
        return request->send(401, "application/json", "{\"message\":\"Invalid OTA password provided!\"}");
      }
    } else Serial.println(F("[OTA] No password configured, no authentication requested!"));

    if (!index) {
      otaIsRunning = true;
      Serial.print(F("[OTA] Begin firmware update with filename: "));
      Serial.println(filename);
      // if filename includes spiffs|littlefs, update the spiffs|littlefs partition
      int cmd = (filename.indexOf("spiffs") > -1 || filename.indexOf("littlefs") > -1) ? U_SPIFFS : U_FLASH;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
        Serial.print(F("[OTA] Error: "));
        Update.printError(Serial);
        request->send(500, "application/json", "{\"message\":\"Unable to begin firmware update!\"}");
        otaIsRunning = false;
      }
    }

    if (Update.write(data, len) != len) {
      Serial.print(F("[OTA] Error: "));
      Update.printError(Serial);
      request->send(500, "application/json", "{\"message\":\"Unable to write firmware update data!\"}");
      otaIsRunning = false;
    }

    if (final) {
      if (!Update.end(true)) {
        String output;
        DynamicJsonDocument doc(32);
        doc["message"] = "Update error";
        doc["error"] = Update.errorString();
        serializeJson(doc, output);
        request->send(500, "application/json", output);

        Serial.println("[OTA] Error when calling calling Update.end().");
        Update.printError(Serial);
        otaIsRunning = false;
      } else {
        Serial.println("[OTA] Firmware update successful.");
        request->send(200, "application/json", "{\"message\":\"Please wait while the device reboots!\"}");
        yield();
        delay(250);

        Serial.println("[OTA] Update complete, rebooting now!");
        Serial.flush();
        ESP.restart();
      }
    }
  });
}

/**
 * @brief Start a background task to regulary check for updates
 */
bool OtaWebUpdater::startBackgroundTask() {
  stopBackgroundTask();
  BaseType_t xReturned = xTaskCreatePinnedToCore(
    otaTask,
    "OtaWebUpdater",
    4000,   // Stack size in words
    this,   // Task input parameter
    0,      // Priority of the task
    &otaCheckTask,  // Task handle.
    0       // Core where the task should run
  );
  if( xReturned != pdPASS ) {
    Serial.println(F("[OTAWEBUPDATER] Unable to run the background Task"));
    return false;
  }
  return true;
}

/**
 * @brief Stops a background task if existing
 */
void OtaWebUpdater::stopBackgroundTask() {
  if (otaCheckTask != NULL) { // make sure there is no task running
    vTaskDelete(otaCheckTask);
    Serial.println(F("[OTAWEBUPDATER] Stopped the background Task"));
  }
}

/**
 * @brief Background Task running as a loop forever
 * @param param needs to be a valid OtaWebUpdater instance
 */
void otaTask(void* param) {
  yield();
  delay(1500); // Do not execute immediately
  yield();

  OtaWebUpdater * otaWebUpdater = (OtaWebUpdater *) param;
  for(;;) {
    yield();
    otaWebUpdater->loop();
    yield();
    vTaskDelay(otaWebUpdater->xDelay);
  }
}

/**
 * @brief Run our internal routine
 */
void OtaWebUpdater::loop() {
  if (newReleaseAvailable) executeUpdate();

  if (networkReady) {
    if (initialCheck) {
      if (millis() - lastVersionCheckMillis < intervalVersionCheckMillis) return;
      lastVersionCheckMillis = millis();
    } else initialCheck = true;

    Serial.println(F("[OTAWEBUPDATER] Searching a new firmware release"));
    checkAvailableVersion();
  }
}

/**
 * @brief Execute the version check from the external Webserver
 * @return true if the check was successfull
 * @return false on error
 */
bool OtaWebUpdater::checkAvailableVersion() {
  WiFiClient client;
  HTTPClient http;
  
  // Send request
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.useHTTP10(true);
  http.begin(client, baseUrl + "/current-version.json");
  http.GET();

  // Parse response
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, http.getStream());

  // Disconnect
  http.end();

  auto date = doc["date"].as<String>();
  auto revision = doc["revision"].as<String>();

  if (date.isEmpty() || revision.isEmpty() || date == "null" || revision == "null") {
    Serial.printf("[OTAWEBUPDATER] Invalid response or json in %s/current-version.json\n", baseUrl.c_str());
    return false;
  }
  if (date > currentFwDate) { // a newer Version is available!
    Serial.printf("[OTAWEBUPDATER] Newer firmware available: %s vs %s\n", date.c_str(), currentFwDate.c_str());
    newReleaseAvailable = true;
  }
  Serial.println(F("[OTAWEBUPDATER] No newer firmware available"));
  return true;
}

/**
 * @brief Download a file from a url and execute the firmware update
 * 
 * @param baseUrl HTTPS url to download from
 * @param filename  The filename to download
 * @return true 
 * @return false 
 */
bool OtaWebUpdater::updateFile(String baseUrl, String filename) {
  otaIsRunning = true;
  int filetype = (filename.indexOf("spiffs") > -1 || filename.indexOf("littlefs") > -1) ? U_SPIFFS : U_FLASH;

  String firmwareUrl = baseUrl + "/" + filename;
  WiFiClient client;
  HTTPClient http;
  
  // Reserve some memory to download the file
  auto bufferAllocationLen = 128*1024;
  uint8_t * buffer;
  try {
    buffer = new uint8_t[bufferAllocationLen];
  } catch (std::bad_alloc& ba) {
    Serial.printf("[OTAWEBUPDATER] Unable to request memory with malloc(%u)\n", bufferAllocationLen+1);
    otaIsRunning = false;
    return false;
  }
  
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.begin(client, firmwareUrl);

  Serial.printf("[OTAWEBUPDATER] Firmware type: %s\n", filetype == U_SPIFFS ? "spiffs" : "flash");
  Serial.printf("[OTAWEBUPDATER] Firmware url:  %s\n", firmwareUrl.c_str());

  if (http.GET() == 200) {
    // get length of document (is -1 when Server sends no Content-Length header)
    auto totalLength = http.getSize();
    auto len = totalLength;
    auto currentLength = 0;

    // this is required to start firmware update process
    Update.begin(UPDATE_SIZE_UNKNOWN, filetype);
    Serial.printf("[OTAWEBUPDATER] Firmware size: %u\n", totalLength);

    // create buffer for read
    //uint8_t buff[4096] = { 0 };
    WiFiClient * stream = http.getStreamPtr();

    // read all data from server
    Serial.println("[OTAWEBUPDATER] Begin firmware upgrade...");
    while(http.connected() && (len > 0 || len == -1)) {
      // get available data size
      size_t size = stream->available();
      if(size) {
        // read up to 4096 byte
        int readBufLen = stream->readBytes(buffer, ((size > bufferAllocationLen) ? bufferAllocationLen : size));
        if(len > 0) len -= readBufLen;

        Update.write(buffer, readBufLen);
        Serial.printf("[OTAWEBUPDATER] Status: %u\n", currentLength);

        currentLength += readBufLen;
        if(currentLength != totalLength) continue;
        // Update completed
        Update.end(true);
        http.end();
        Serial.println();
        Serial.printf("[OTAWEBUPDATER] Upgrade successfully executed. Wrote bytes: %u\n", currentLength);

        otaIsRunning = false;
        delete[] buffer;
        return true;
      }
      delay(1);
    }
  }

  otaIsRunning = false;
  delete[] buffer;
  return false;
}

/**
 * @brief Execute the update with a firmware from the external Webserver
 */
void OtaWebUpdater::executeUpdate() {
  otaIsRunning = true;
  if (updateFile(baseUrl, "littlefs.bin") && updateFile(baseUrl, "firmware.bin") ) {
    ESP.restart();
  } else {
    otaIsRunning = false;
    Serial.println("[OTAWEBUPDATER] Failed to update firmware");
  }
}
