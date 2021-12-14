/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2021 Martin Verges
 *
**/

#include <Arduino.h>
#include <HX711.h>
#include <Preferences.h>
#include "sensor.h"

HX711 hx711;
Preferences preferences;

TANKLEVEL::TANKLEVEL() {
}

TANKLEVEL::~TANKLEVEL() {
}

void TANKLEVEL::begin(uint8_t dout, uint8_t pd_sck, String nvs) {
    hx711.begin(dout, pd_sck, 32);
    NVS_NAMESPACE = nvs;

    if (!preferences.begin(NVS_NAMESPACE.c_str(), false)) {
        Serial.println("Error opening NVS Namespace, giving up...");
    } else {
        levelConfig.setupDone = preferences.getBool("setupDone", false);
        if (levelConfig.setupDone) {
            Serial.println("LevelData restored from Storage...");
            for (uint8_t i = 0; i < 100; i++) {
                levelConfig.readings[i] = (float)preferences.getInt(String("val" + String(i)).c_str(), 0);
            }
        } else {
            Serial.println("No stored configuration found on NVS...");
        }
        preferences.end();
    }
}

bool TANKLEVEL::isSetupRunning() {
    return setupConfig.readings[0] > 0;
}

bool TANKLEVEL::isConfigured() {
    return levelConfig.readings[0] > 0;
}

int TANKLEVEL::getMedian() {
    return (int)hx711.get_median_value(10);
}

int TANKLEVEL::getPercentage() {
    int val = TANKLEVEL::getMedian();
    for(int x=99; x>0; x--) {
        if (val > levelConfig.readings[x]) return x+1;
    }
    return 0;
}

void TANKLEVEL::setLimits(float lower_end, float upper_end) {
    LOWER_END = lower_end;
    UPPER_END = upper_end;
}

void TANKLEVEL::printData(int* readings, size_t count) {
  for (uint8_t i = 0; i < count; i++) Serial.printf("%d = %d\n", i, readings[i]);
}

void TANKLEVEL::printData() {
    TANKLEVEL::printData(levelConfig.readings, 100);
}

/*
UNITS = one reading: 4768.1  | average:  4768.9
VALUE = one reading:  477464.0  | average:  476669.0
RAW   = one reading:  4316381 | average:  4315608
*/
void TANKLEVEL::readPressure() {
  if (hx711.wait_ready_retry(100)) {
    currentState = getPercentage();
    Serial.printf("Tank fill status = %d\n", currentState);
  } else {
    Serial.println("Unable to communicate with the HX711 modul.");
  }
}

// Search through the setupConfig sensor readings and find the upper limit cutoff index
int TANKLEVEL::findEndCutoffIndex() {
  float endCutoff = (setupConfig.readings[MAX_DATA_POINTS-1] - setupConfig.readings[0]) * UPPER_END + setupConfig.readings[0];
  int endIndex = -1;
  for (uint8_t i = 0; i < MAX_DATA_POINTS; i++) {
    if (setupConfig.readings[i] > endCutoff) {
      endIndex = i;
      break;
    }
  }
  return endIndex;
}

// Search through the setupConfig sensor readings and find the lower limit cutoff index
int TANKLEVEL::findStartCutoffIndex(int endIndex) {
  float startCutoff = (setupConfig.readings[endIndex] - setupConfig.readings[0]) * LOWER_END + setupConfig.readings[0];
  int startIndex = -1;
  for (uint8_t i = 0; i < MAX_DATA_POINTS; i++) {
    if (setupConfig.readings[i] < startCutoff) startIndex = i;
    else break;
  }
  return startIndex;
}

void TANKLEVEL::endLevelSetup() {
    Serial.println("Exiting setup");

    while(setupConfig.valueCount < MAX_DATA_POINTS) {
        setupConfig.readings[setupConfig.valueCount++] = setupConfig.lastread;
    }

    std::sort(std::begin(setupConfig.readings), std::end(setupConfig.readings));
    // printData(setupConfig.readings, MAX_DATA_POINTS);
    int endIndex = findEndCutoffIndex();
    int startIndex = findStartCutoffIndex(endIndex);
    // Serial.printf("Start Index = %d\n", startIndex);
    // Serial.printf("End Index = %d\n", endIndex);

    // calculate the percentages
    int readingCount = endIndex - startIndex + 1;  // number of readings to use
    int x = startIndex;                        // index of reading[]
    float y1 = 0.00;                           // lower percentage of reading[] index (e.g. entry x equals 55%)
    float y2 = 0.00;                           // upper percentage of reading[] index (e.g. entry x+1 equals 58%)
    for (size_t Y = 0; Y <=100; Y++) {         // % value (1-100%)
      if (Y==0) {
        levelConfig.readings[Y++] = setupConfig.readings[x];
        continue;
      }
      while (Y > y2 && x <= endIndex) {
        // find the next dataset to calculate Z
        y1 = (float)(x-startIndex  ) / readingCount * 100;     // lower percentage of this sensor reading
        y2 = (float)(x-startIndex+1.0) / readingCount * 100;     // upper percentage of the next sensor reading
        x++;
      }
      // float Z = setupConfig.readings[x-1] + ((Y - y1) / (y2 - y1) * (setupConfig.readings[x] - setupConfig.readings[x-1])); // save the value into the configuration
      levelConfig.readings[Y] = setupConfig.readings[x-1] + ((Y - y1) / (y2 - y1) * (setupConfig.readings[x] - setupConfig.readings[x-1]));
      // Serial.printf("\t\t\ty1=\t%f\t | y2=\t%f\n", y1, y2);
      // Serial.printf("Y=%d\t| Z = %d | z1=\t%f\t| z2=\t%f\n", Y, (int)Z, setupConfig.readings[x-1], setupConfig.readings[x]);
    }
    // printData(levelConfig.readings, 100);
    levelConfig.setupDone = true;
    
    preferences.begin(NVS_NAMESPACE.c_str(), false); 
    preferences.clear();
    preferences.putBool("setupDone", true);
    for (uint8_t i = 0; i < 100; i++) {
      preferences.putInt(String("val" + String(i)).c_str(), levelConfig.readings[i]);
    }
    preferences.end();

    setupConfig.readings[0] = 0;
    setupConfig.valueCount = 0;
    return;
}

void TANKLEVEL::levelSetup() {
  if (setupConfig.readings[0] <= 0) {  // Start the level setup
    setupConfig.valueCount = 0;
    setupConfig.readings[setupConfig.valueCount++] = hx711.get_max_value(10);
    Serial.printf("Begin level setup with minValue of %d\n", setupConfig.readings[0]);
    return;
  }
  if (setupConfig.valueCount >= MAX_DATA_POINTS) {
    
  }
  setupConfig.lastread = (int)hx711.get_median_value(10);
  Serial.printf("Recording new entry with a value of %d\n", setupConfig.lastread);
  setupConfig.readings[setupConfig.valueCount++] = setupConfig.lastread;
}
