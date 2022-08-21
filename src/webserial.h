/**
 * @file webserial.h
 * @author Martin Verges <martin@veges.cc>
 * @version 0.1
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/gaslevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#ifndef WEBSERIAL_h
#define WEBSERIAL_h

#include <ESPAsyncWebServer.h>

class WebSerialClass {
    public:
        void begin(AsyncWebServer *server, const char* url = "/api/webserial");

        void print(int c);
        void print(uint8_t c);
        void print(uint16_t c);
        void print(uint32_t c);
        void print(double c);
        void print(float c);
        void print(const char * c);
        void print(char * c);
        void print(String c);

        void println(int c);
        void println(uint8_t c);
        void println(uint16_t c);
        void println(uint32_t c);
        void println(float c);
        void println(double c);
        void println(const char * c);
        void println(char * c);
        void println(String c);

        size_t printf(const char *format, ...);

    private:
        AsyncWebSocket * webSocket;
        AsyncWebServer * webServer = nullptr;
};

#endif // WEBSERIAL_h
