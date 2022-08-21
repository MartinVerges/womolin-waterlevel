

#include "webserial.h"
extern WebSerialClass WebSerial;

#ifndef LOG_INFO
  #define LOG_INFO(...)  do {     \
    Serial.print(__VA_ARGS__);    \
    WebSerial.print(__VA_ARGS__); \
    } while(0)
#endif // LOG_INFO(...)

#ifndef LOG_INFO_LN
  #define LOG_INFO_LN(...) do {    \
    Serial.println(__VA_ARGS__);   \
    WebSerial.println(__VA_ARGS__);  \
    } while(0)
#endif // LOG_INFO_LN(...)

#ifndef LOG_INFO_F
  #define LOG_INFO_F(format, ...)  do {     \
    Serial.printf(format, __VA_ARGS__);    \
    WebSerial.printf(format, __VA_ARGS__);  \
    } while(0)
#endif // LOG_INFO_F(format, ...)
