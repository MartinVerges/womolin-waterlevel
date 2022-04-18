/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#ifndef DAC_MIN_MVOLT
#define DAC_MIN_MVOLT 500.0                 // DAC output minimum value (~0.5V on 0% tank level)
#endif
#ifndef DAC_MAX_MVOLT
#define DAC_MAX_MVOLT 3000.0                // DAC output maximum value (~3V on 100% tank level)
#endif
#ifndef DAC_VCC
#define DAC_VCC 3300.0                      // DAC output maximum voltage from esp32 3.3V = 3300mV
#endif

#include <Arduino.h>
#include <driver/dac.h>

bool enableDac = true;                      // Disable it if you don't need an analog output

// Set the current tank level value to the DAC output
uint8_t dacValue(uint8_t use_dac, uint8_t percentage) {
  if (!enableDac) return 0;

  dac_channel_t channel;
  switch (use_dac) {
    case 1: channel = DAC_CHANNEL_1; break;
    case 2: channel = DAC_CHANNEL_2; break;
    default:
      Serial.println("[ERROR] DAC Channel not found!");
      return -1;
  }

  uint8_t val = 0;
  if (percentage >= 0 && percentage <= 100) {
    float start = DAC_MIN_MVOLT / DAC_VCC * 255;   // startvolt / maxvolt * datapoints
    float end = DAC_MAX_MVOLT / DAC_VCC * 255;     // endvolt / maxvolt * datapoints
    val = round(start + (end-start) / 100.0 * percentage);
    dac_output_enable(channel);
    dac_output_voltage(channel, val);
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", val, (float)DAC_VCC/255*val);
  } else {
    dac_output_enable(channel);
    dac_output_voltage(channel, 0);
    Serial.printf("[GPIO] DAC output set to %d or %.2fmV\n", 0, 0.00);
  }
  return val;
}
