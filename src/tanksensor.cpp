// Fix an issue with the HX711 library on ESP32
#undef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32 true

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HX711.h>
#include <LITTLEFS.h>
#include <Preferences.h>
#include <WiFi.h>

#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif

#define NVS_NAMESPACE "tanksensor"

const int webserverPort = 80;
String wifi_ssid = "TankSensor";
String wifi_pass = "SuperSecret";
String hostName = "my-tanksensor";

HX711 hx711;
Preferences preferences;

struct Button {
  const uint8_t PIN;
  bool pressed;
};

Button button1 = {21, false};              // Run the setup 
Button button2 = {22, false};              // unused

const int GPIO_HX711_DOUT = 26;            // GPIO pin to use for DT or DOUT
const int GPIO_HX711_SCK = 25;             // GPIO pin to use for SCK

const float LOWER_END = 0.010;             // value increase to start recording (tank is empty)
const float UPPER_END = 0.990;             // value limit to cutoff data (tank is full)

struct config_t {
  bool setupDone = false;                  // Configuration done or not yet initialized sensor
  int readings[100] = {0};                 // pressure readings to map to percentage filling
} levelConfig;
int currentState = 0;                      // last reading in percent
unsigned long millisStarted = 0;           // timer to run parts only at given interval

const int MAX_DATA_POINTS = 255;           // how many level data points to store (increased accuracy)
struct state_t {
  int valueCount = 0;                      // current number of entries in readings[]
  int minValue = 0.00;                     // lowest value to start recording
  int lastread = 0.00;                     // last redading while in setup  
  int readings[MAX_DATA_POINTS] = {0};     // data readings from pressure sensor while running level setup
} setupConfig;

AsyncWebServer server(webserverPort);

void printData(int* readings, size_t count) {
  for (uint8_t i = 0; i < count; i++) Serial.printf("%d = %d\n", i, readings[i]);
}

int getPercentage(float val) {
  for(int x=99; x>0; x--) {
    if (val > levelConfig.readings[x]) return x+1;
  }
  return 0;
}

// Search through the setupConfig sensor readings and find the upper limit cutoff index
int findEndCutoffIndex() {
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
int findStartCutoffIndex(int endIndex) {
  float startCutoff = (setupConfig.readings[endIndex] - setupConfig.readings[0]) * LOWER_END + setupConfig.readings[0];
  int startIndex = -1;
  for (uint8_t i = 0; i < MAX_DATA_POINTS; i++) {
    if (setupConfig.readings[i] < startCutoff) startIndex = i;
    else break;
  }
  return startIndex;
}

void levelSetup() {
  if (setupConfig.readings[0] <= 0) {  // Start the level setup
    setupConfig.valueCount = 0;
    setupConfig.readings[setupConfig.valueCount++] = hx711.get_max_value(10);
    Serial.printf("Begin level setup with minValue of %d\n", setupConfig.readings[0]);
    return;
  }
  if (setupConfig.valueCount >= MAX_DATA_POINTS) {
    Serial.println("Exiting setup");

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
    
    preferences.begin(NVS_NAMESPACE, false); 
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
  setupConfig.lastread = (int)hx711.get_median_value(10);
  Serial.printf("Recording new entry with a value of %d\n", setupConfig.lastread);
  setupConfig.readings[setupConfig.valueCount++] = setupConfig.lastread;
}

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}
void IRAM_ATTR ISR_button2() {
  button2.pressed = true;
}

void loadFileContent() {
  File file = LITTLEFS.open("/index.html");
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
  return;
}

void setup() {
  Serial.begin(115200);
  hx711.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, 32);
  
  if(!LITTLEFS.begin(true)){
    Serial.println("An Error has occurred while mounting LITTLEFS");
    ESP.restart();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wifi_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LITTLEFS, "/index.html", String(), false);
  });
  server.on("/api/setup", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Future: start/end setup");
  });
  server.on("/api/reading/raw", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", (String)(int)hx711.get_median_value(10));
  });
  server.on("/api/level", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", (String)getPercentage(hx711.get_median_value(10)));
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  if (!preferences.begin(NVS_NAMESPACE, false)) {
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

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, ISR_button2, FALLING);
}

/*
UNITS = one reading: 4768.1  | average:  4768.9
VALUE = one reading:  477464.0  | average:  476669.0
RAW   = one reading:  4316381 | average:  4315608
 */
void readPressure() {
  if (hx711.wait_ready_retry(100)) {
    currentState = getPercentage(hx711.get_median_value(10));
    Serial.printf("Tank fill status = %d\n", currentState);
  } else {
    Serial.println("Unable to communicate with the HX711 modul.");
  }
}

void loop() {
  if (button1.pressed) {
    if (setupConfig.readings[0] > 0) {
      Serial.println("User requested end of setup...");
      while(setupConfig.valueCount < MAX_DATA_POINTS) {
        setupConfig.readings[setupConfig.valueCount++] = setupConfig.lastread;
      }
    }
    levelSetup();
    button1.pressed = false;
  }

  if (button2.pressed) {
    button2.pressed = false;
    printData(levelConfig.readings, 100);
  }
  
  if (setupConfig.readings[0] > 0) {
    // run the level setup
    if (millis() - millisStarted >= 100) {
      millisStarted = millis();
      levelSetup();
    }
  } else {
    // run regular operation
    if (millis() - millisStarted > 1000) {
      millisStarted = millis();
      if (levelConfig.readings[0] > 0) readPressure();
      else Serial.println("No leveldata found, please run the setup to use the device!");
    }
  }
  delay(25);
}
