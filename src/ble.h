/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#define BLE_SERVICE_LEVEL "2AF9"            // Bluetooth LE service ID for tank level
#define BLE_CHARACTERISTIC_LEVEL "181A"     // Bluetooth LE characteristic ID for tank level value

#include <Arduino.h>

extern bool enableBle;

void createBleServer(String hostname);
void updateBleCharacteristic(int val);
