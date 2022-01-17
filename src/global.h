#include <Arduino.h>
#include "tanklevel.h"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <esp_bt_device.h>
#include <FS.h>
#include <NimBLEDevice.h>
#include <NimBLEBeacon.h>
#include <Preferences.h>
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}
#define SPIFFS LITTLEFS 
#include <LITTLEFS.h>

#define GPIO_HX711_DOUT 33                  // GPIO pin to use for DT or DOUT
#define GPIO_HX711_SCK 32                   // GPIO pin to use for SCK
#define webserverPort 80                    // Start the Webserver on this port
#define DAC_MIN_MVOLT 500.0                 // DAC output minimum value (0.5V on 0% tank level)
#define DAC_MAX_MVOLT 3000.0                // DAC output maximum value (3V on 100% tank level)
#define DAC_VCC 3300.0                      // DAC output maximum voltage from esp32 3.3V = 3300mV
#define NVS_NAMESPACE "tanksensor"          // Preferences.h namespace to store settings
#define BLE_SERVICE_LEVEL "2AF9"            // Bluetooth LE service ID for tank level
#define BLE_CHARACTERISTIC_LEVEL "181A"     // Bluetooth LE characteristic ID for tank level value

RTC_DATA_ATTR struct timing_t {
  // Wifi interval in loop()
  uint64_t lastWifiCheck = 0;               // last millis() from WifiCheck
  const unsigned int wifiInterval = 30000;  // Interval in ms to execute code

  // Sensor data in loop()
  uint64_t lastSensorRead = 0;              // last millis() from Sensor read
  const unsigned int sensorInterval = 1000; // Interval in ms to execute code

  // Setup executing in loop()
  uint64_t lastSetupRead = 0;               // last millis() from Setup run
  const unsigned int setupInterval = 250;   // Interval in ms to execute code
} Timing;

bool enableWifi = false;                    // Enable Wifi, disable to reduce power consumtion, stored on NVS

RTC_DATA_ATTR bool startWifiConfigPortal = false; // Start the config portal on setup() (default set by wakeup funct.)
RTC_DATA_ATTR uint64_t sleepTime = 0;             // Time that the esp32 slept

TANKLEVEL Tanklevel;

struct Button {
  const gpio_num_t PIN;
  bool pressed;
};
Button button1 = {GPIO_NUM_4, false};       // Run the setup (use a RTC GPIO)

String hostName;
uint32_t blePin;
DNSServer dnsServer;
AsyncWebServer webServer(webserverPort);
AsyncEventSource events("/events");
Preferences preferences;

static NimBLEServer* pServer;


void MDNSRegister() {
  if (!enableWifi) return;
  Serial.println("[MDNS] Starting mDNS Service!");
  MDNS.begin(hostName.c_str());
  MDNS.addService("http", "tcp", 80);
}

String getMacFromBT(String spacer = "") {
  String output = "";
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  for (int i = 0; i < 6; i++) {
    char m[3];
    sprintf(m, "%02X", mac[i]);
    output += m;
    if (i < 5) output += spacer;
  }
  return output;
}

void createBleServer() {
  Serial.println(F("[BLE] Initializing the Bluetooth low energy (BLE) stack"));
  NimBLEDevice::init(hostName.c_str());
  NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);
  pServer = NimBLEDevice::createServer();

  // BLE Environmental Service (haven't found a better one)
  NimBLEService *pEnvService = pServer->createService(BLE_SERVICE_LEVEL);
  NimBLECharacteristic *pCharacteristicLevel = pEnvService->createCharacteristic(BLE_CHARACTERISTIC_LEVEL, // Generic Level
    NIMBLE_PROPERTY::READ |
    NIMBLE_PROPERTY::BROADCAST |
    NIMBLE_PROPERTY::NOTIFY |
    NIMBLE_PROPERTY::INDICATE
  );
  NimBLE2904* p2904 = (NimBLE2904*)pCharacteristicLevel->createDescriptor("2904"); 
  p2904->setFormat(NimBLE2904::FORMAT_UINT8);
  p2904->setUnit(NimBLE2904::FORMAT_UINT8);

  pEnvService->start();
  pCharacteristicLevel->setValue(0);
  
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  Serial.print(F("[BLE] Begin Advertising of "));
  Serial.println(pEnvService->getUUID().toString().c_str());
  pAdvertising->addServiceUUID(pEnvService->getUUID());
  pAdvertising->setScanResponse(true); // false will reduce power consumtion
  pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_DIR);
  pAdvertising->start();

  Serial.println(F("[BLE] Advertising Started"));
}

void updateBleCharacteristic(int val) {
  if (pServer->getConnectedCount()) {
    NimBLEService* pSvc = pServer->getServiceByUUID(BLE_SERVICE_LEVEL);
    if(pSvc) {
        NimBLECharacteristic* pChr = pSvc->getCharacteristic(BLE_CHARACTERISTIC_LEVEL);
        if(pChr) {
            pChr->setValue(val);
            pChr->notify(true);
        }
    }
  }
}
