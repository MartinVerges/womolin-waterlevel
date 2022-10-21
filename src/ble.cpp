/**
 * @file ble.cpp
 * @author Martin Verges <martin@verges.cc>
 * @version 0.1
 * @date 2022-07-09
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/waterlevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#include "log.h"

#include "ble.h"
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

bool enableBle = true;                      // Enable Ble, disable to reduce power consumtion, stored in NVS

void stopBleServer() {
  NimBLEDevice::deinit();
}

void createBleServer(String hostname) {
  LOG_INFO_LN(F("[BLE] Initializing the Bluetooth low energy (BLE) stack"));
  NimBLEDevice::init(hostname.c_str());
  //NimBLEDevice::setPower(ESP_PWR_LVL_P9, ESP_BLE_PWR_TYPE_ADV);
  //NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
  //NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);
  pServer = NimBLEDevice::createServer();

  // BLE Environmental Service (haven't found a better one)
  NimBLEService *pEnvService = pServer->createService(BLE_SERVICE_LEVEL);
  NimBLECharacteristic *pCharacteristicLevel = pEnvService->createCharacteristic(BLE_CHARACTERISTIC_LEVEL, // Generic Level
    NIMBLE_PROPERTY::READ |
    NIMBLE_PROPERTY::BROADCAST |
    NIMBLE_PROPERTY::NOTIFY 
    //NIMBLE_PROPERTY::INDICATE
  );
  NimBLE2904* p2904 = (NimBLE2904*)pCharacteristicLevel->createDescriptor("2904"); 
  p2904->setFormat(NimBLE2904::FORMAT_UINT8);
  p2904->setUnit(NimBLE2904::FORMAT_UINT8);

  pEnvService->start();
  pCharacteristicLevel->setValue(0);
  
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  LOG_INFO(F("[BLE] Begin Advertising of "));
  LOG_INFO_LN(pEnvService->getUUID().toString().c_str());
  pAdvertising->addServiceUUID(pEnvService->getUUID());
  //pAdvertising->setScanResponse(true); // false will reduce power consumtion
  //pAdvertising->setAdvertisementType(BLE_GAP_CONN_MODE_DIR);
  NimBLEDevice::startAdvertising();
  LOG_INFO_LN(F("[BLE] Advertising Started"));
}

// FIXME: need to manage multiple levels given by "ch"
void updateBleCharacteristic(uint8_t ch, int val) {
  if (pServer->getConnectedCount()) {
    NimBLEService* pSvc = pServer->getServiceByUUID(BLE_SERVICE_LEVEL);
    if(pSvc) {
        NimBLECharacteristic* pChr = pSvc->getCharacteristic(BLE_CHARACTERISTIC_LEVEL);
        if(pChr) {
            LOG_INFO(F("[BLE] set value (and notify) to "));
            LOG_INFO_LN(val);
            pChr->setValue(val);
            pChr->notify(true);
        }
    }
  }
}
