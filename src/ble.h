/**
 * @file ble.h
 * @author Martin Verges <martin@veges.cc>
 * @version 0.1
 * @date 2022-07-09
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/waterlevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#define BLE_SERVICE_LEVEL "2AF9"            // Bluetooth LE service ID for tank level
#define BLE_CHARACTERISTIC_LEVEL "181A"     // Bluetooth LE characteristic ID for tank level value

#include <Arduino.h>

extern bool enableBle;

void stopBleServer();
void createBleServer(String hostname);
void updateBleCharacteristic(int val);
