/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#include <Arduino.h>
#include <HX711.h>
#include <Preferences.h>
#include "tanklevel.h"
#include <soc/rtc.h>
extern "C" {
  #include <esp_clk.h>
}

// Store some history in the RTC RAM that survives deep sleep
#define MAX_RTC_HISTORY 100                        // store some data points to provide short term history
typedef struct {
    int currentLevel;
    int sensorValue;
    uint64_t timestamp;
} sensorReadings;
RTC_DATA_ATTR sensorReadings readingHistory[MAX_RTC_HISTORY];
RTC_DATA_ATTR int readingHistoryCount = 0;

TANKLEVEL::TANKLEVEL(uint8_t dout, uint8_t pd_sck) {
    hx711.begin(dout, pd_sck, 32);
}

TANKLEVEL::~TANKLEVEL() {
}

uint64_t TANKLEVEL::runtime() {
  return rtc_time_slowclk_to_us(rtc_time_get(), esp_clk_slowclk_cal_get()) / 1000;
}

bool TANKLEVEL::writeToNVS() {
  if (preferences.begin(NVS.c_str(), false)) {
    preferences.clear();
    preferences.putBool("setupDone", true);
    for (uint8_t i = 0; i <= 100; i++) {
      preferences.putInt(String("val" + String(i)).c_str(), levelConfig.readings[i]);
      // Serial.print("Write new value = ");
      // Serial.println(levelConfig.readings[i]);
    }
    preferences.end();
    return true;
  } else {
    Serial.println("Unable to write data to NVS, giving up...");
    return false;
  }
}

bool TANKLEVEL::writeSingleEntrytoNVS(uint8_t i, int value) {
  if (i == 255 && preferences.begin(NVS.c_str(), false)) {
    preferences.putBool("setupDone", value > 0);
    preferences.end();
    return true;
  } else if (i < 0 or i > 100) return false;
  if (preferences.begin(NVS.c_str(), false)) {
    preferences.putInt(String("val" + String(i)).c_str(), value);
    preferences.end();
    return true;
  }
  return false;
}

int TANKLEVEL::getLevelData(int perc) {
  if (perc >= 0 and perc <= 100) {
    return levelConfig.readings[perc];
  } else return -1;
}

void TANKLEVEL::begin(String ns) {
  NVS = ns;

  if (!preferences.begin(NVS.c_str(), false)) {
    Serial.println("Error opening NVS Namespace, giving up...");
  } else {
    levelConfig.setupDone = preferences.getBool("setupDone", false);
    if (levelConfig.setupDone) {
      Serial.println("LevelData restored from Storage...");
      for (uint8_t i = 0; i <= 100; i++) {
        levelConfig.readings[i] = preferences.getInt(String("val" + String(i)).c_str(), 0);
      }
    } else {
      Serial.println("No stored configuration found on NVS...");
    }
    preferences.end();
  }
}

bool TANKLEVEL::isSetupRunning() {
  if (setupConfig.start) return beginLevelSetup();
  return setupConfig.readings[0] > 0;
}

bool TANKLEVEL::isConfigured() {
  if (setupConfig.start) { beginLevelSetup(); return false; }
  return levelConfig.readings[0] > 0;
}

int TANKLEVEL::getSensorMedianValue(bool cached) {
  if (cached) return lastMedian;
  if (hx711.wait_ready_retry(100, 5)) {
    lastMedian = (int)floor(hx711.get_median_value(10) / 1000);
    return lastMedian;
  } else {
    Serial.println("Unable to communicate with the HX711 modul.");
    return -1;
  }
}

int TANKLEVEL::getCalculatedPercentage(bool cached) {
  int val = lastMedian;
  if (!cached && val == 0) {
    val = getSensorMedianValue(false);
  } else if (!cached) {
    //Serial.print("attempt val = ");
    //Serial.print(val);
    // This is an attempt to reduce strong fluctuations (e.g. while driving).
    val = (val * 10 + getSensorMedianValue(false)) / 11;
    //Serial.print(" newval = ");
    //Serial.println(val);
  }
  for(int x=100; x>0; x--) {
    if (val >= levelConfig.readings[x]) {
      if (!cached) {
        // Write a new entry to readingHistory
        readingHistory[readingHistoryCount].currentLevel = x;
        readingHistory[readingHistoryCount].sensorValue = val;
        readingHistory[readingHistoryCount].timestamp = runtime();
        readingHistoryCount++;
        if (readingHistoryCount >= MAX_RTC_HISTORY) { // build a ringbuffer and overwrite the oldest entry
          readingHistoryCount = 0;
          for (int r = 0; r < MAX_RTC_HISTORY; r++){
            Serial.printf("%d,%d,%" PRIu64 "\n",
              readingHistory[r].currentLevel,
              readingHistory[r].sensorValue,
              readingHistory[r].timestamp
            );
          }
        }
      }
      return x;
    }
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
    printData(levelConfig.readings, 101);
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

bool TANKLEVEL::beginLevelSetup() {
  setupConfig.start = false;
  if (!isSetupRunning()) {  // Start the level setup
    setupConfig.valueCount = 0;
    setupConfig.readings[setupConfig.valueCount++] = getSensorMedianValue(false);
    Serial.printf("Begin level setup with minValue of %d\n", setupConfig.readings[0]);
    return true;
  } else {
    Serial.println("Level setup is already running");
    return false;
  }
}

void TANKLEVEL::setStartAsync() {
  setupConfig.start = true;
}

void TANKLEVEL::setEndAsync() {
  setupConfig.end = true;
}

void TANKLEVEL::setAbortAsync() {
  setupConfig.abort = true;
}

bool TANKLEVEL::abortLevelSetup() {
  Serial.print("Abort setup ... ");
  setupConfig.valueCount = 0;
  while(setupConfig.valueCount < MAX_DATA_POINTS) { // cleanup
    setupConfig.readings[setupConfig.valueCount++] = 0;
  }
  resetSetupData();
  Serial.println("done");
  return true;
}

void TANKLEVEL::resetSetupData() {
  setupConfig.readings[0] = 0;
  setupConfig.valueCount = 0;
  setupConfig.start = false;
  setupConfig.abort = false;
  setupConfig.end = false;
}

bool TANKLEVEL::setupFrom2Values(int lower, int upper) {    
  if (upper < lower) return false;
  for (size_t Y = 0; Y <= 100; Y++) {
    levelConfig.readings[Y] = (upper - lower) / 100.0 * Y + lower;
    //Serial.println(levelConfig.readings[Y]);
  }
  if (levelConfig.readings[0] == lower && levelConfig.readings[100] == upper) {
    Serial.println("Level config done!"); 
    levelConfig.setupDone = true;
    writeToNVS();
    return true;
  } else {
    Serial.printf("Written data differs (%d vs %d) and (%d vs %d) to given values! Giving up!\n", 
      levelConfig.readings[0], lower,
      levelConfig.readings[100], upper
    ); 
    levelConfig.setupDone = false;
    return false;
  }
}

bool TANKLEVEL::endLevelSetup() {
  if (setupConfig.abort) return abortLevelSetup();
  Serial.println("Exiting setup");

  while(setupConfig.valueCount < MAX_DATA_POINTS) {
    // force end of setup by filling empty slots
    setupConfig.readings[setupConfig.valueCount++] = lastMedian;
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
  for (size_t Y = 0; Y <= 100; Y++) {         // % value (1-100%)
    if (Y==0) {
      levelConfig.readings[Y] = setupConfig.readings[x];
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
  
  if (!writeToNVS()) return false;

  // cleanup
  resetSetupData();
  return true;
}

int TANKLEVEL::runLevelSetup() {
  if (setupConfig.start) return beginLevelSetup();
  if (setupConfig.abort) return abortLevelSetup();
  if (setupConfig.end) return endLevelSetup();
  if (isSetupRunning()) {
    if (setupConfig.valueCount >= MAX_DATA_POINTS) {
      endLevelSetup();
      return 0;
    }
    Serial.printf("Recording new entry with a value of %d\n", getSensorMedianValue(false));
    setupConfig.readings[setupConfig.valueCount++] = getSensorMedianValue(true);
    return getSensorMedianValue(true);
  } else return 0;
}
