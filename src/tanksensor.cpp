// Fix an issue with the HX711 library on ESP32
#undef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32 true

#include <Arduino.h>
#include <AsyncElegantOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LITTLEFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <sensor.h>

#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
 #include <time.h>
#endif

#define NVS_NAMESPACE "tanksensor"

const int GPIO_HX711_DOUT = 26;            // GPIO pin to use for DT or DOUT
const int GPIO_HX711_SCK = 25;             // GPIO pin to use for SCK
const int webserverPort = 80;              // Start the Webserver on this port

unsigned long millisStarted = 0;           // timer to run parts only at given interval

String wifi_ssid = "TankSensor";
String wifi_pass = "SuperSecret";
String hostName = "my-tanksensor";

TANKLEVEL Tanklevel;

struct Button {
  const uint8_t PIN;
  bool pressed;
};
Button button1 = {21, false};              // Run the setup 
Button button2 = {22, false};              // unused

AsyncWebServer server(webserverPort);

void IRAM_ATTR ISR_button1() {
  button1.pressed = true;
}
void IRAM_ATTR ISR_button2() {
  button2.pressed = true;
}

void setup() {
  Serial.begin(115200);
  Tanklevel.begin(GPIO_HX711_DOUT, GPIO_HX711_SCK, NVS_NAMESPACE);
  
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
    request->send(200, "text/plain", (String)Tanklevel.getMedian());
  });
  server.on("/api/level", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", (String)Tanklevel.getPercentage());
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, ISR_button1, FALLING);
  pinMode(button2.PIN, INPUT_PULLUP);
  attachInterrupt(button2.PIN, ISR_button2, FALLING);
}

void loop() {
  if (button1.pressed) {
    if (Tanklevel.isSetupRunning()) {
      Serial.println("User requested end of setup...");
      Tanklevel.endLevelSetup();
    } else {
      Tanklevel.levelSetup();
    }
    button1.pressed = false;
  }

  if (button2.pressed) {
    button2.pressed = false;
    Tanklevel.printData();
  }
  
  if (Tanklevel.isSetupRunning()) {
    // run the level setup
    if (millis() - millisStarted >= 100) {
      millisStarted = millis();
      Tanklevel.levelSetup();
    }
  } else {
    // run regular operation
    if (millis() - millisStarted > 1000) {
      millisStarted = millis();
      if (Tanklevel.isConfigured()) Tanklevel.readPressure();
      else Serial.println("Tanklevel Sensor not configured, please run the setup!");
    }
  }
  delay(25);
}
