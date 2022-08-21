/**
 * @file webserial.cpp
 * @author Martin Verges <martin@veges.cc>
 * @version 0.1
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/gaslevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#include "log.h"
#include <webserial.h>

void WebSerialClass::begin(AsyncWebServer *server, const char* url) {
  webServer = server;

  webSocket = new AsyncWebSocket("/api/webserial");
  webSocket->onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) -> void {
    if(type == WS_EVT_CONNECT){
      LOG_INFO_LN(F("[WEBSERIAL] Client connection received"));
    } else if(type == WS_EVT_DISCONNECT){
      LOG_INFO_LN(F("[WEBSERIAL] Client disconnected"));
    } else if(type == WS_EVT_DATA){
      LOG_INFO_LN(F("[WEBSERIAL] Received Websocket Data"));
    }
  });
  webServer->addHandler(webSocket);

  LOG_INFO_LN(F("[WEBSERIAL] Attached AsyncWebServer along with Websockets"));
}

void WebSerialClass::print(int c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(uint8_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(uint16_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(uint32_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(double c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(float c) {
  if (webServer != nullptr) webSocket->textAll(String(c));
}

void WebSerialClass::print(const char * c) {
  if (webServer != nullptr) webSocket->textAll(c);
}

void WebSerialClass::print(char * c) {
  if (webServer != nullptr) webSocket->textAll(c);
}

void WebSerialClass::print(String c) {
  if (webServer != nullptr) webSocket->textAll(c);
}

void WebSerialClass::println(int c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(uint8_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(uint16_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(uint32_t c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(float c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(double c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(const char * c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(char * c) {
  if (webServer != nullptr) webSocket->textAll(String(c) + "\n");        
}

void WebSerialClass::println(String c) {
  if (webServer != nullptr) webSocket->textAll(c + "\n");        
}


// Based on LOG_INFO_F() from arduino/esp32 core
size_t WebSerialClass::printf(const char *format, ...) {
  char loc_buf[64];
  char * temp = loc_buf;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
  va_end(copy);
  if (len < 0) {
    va_end(arg);
    return 0;
  };
  if (len >= sizeof(loc_buf)) {
    temp = (char*) malloc(len+1);
    if(temp == NULL) {
      va_end(arg);
      return 0;
    }
    len = vsnprintf(temp, len+1, format, arg);
  }
  va_end(arg);

  print(String((uint8_t*)temp, len));

  if(temp != loc_buf) free(temp);
  return len;
}
