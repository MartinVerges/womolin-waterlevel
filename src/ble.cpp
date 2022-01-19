/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include "ble.h"
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

bool enableBle = true;                      // Enable Ble, disable to reduce power consumtion, stored in NVS

void stopBleServer() {
  NimBLEDevice::deinit();
}

void createBleServer(String hostname) {
  Serial.println(F("[BLE] Initializing the Bluetooth low energy (BLE) stack"));
  NimBLEDevice::init(hostname.c_str());
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
