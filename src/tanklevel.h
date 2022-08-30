/**
 * @file tanklevel.h
 * @author Martin Verges <martin@veges.cc>
 * @version 0.1
 * @date 2022-07-09
 * 
 * @copyright Copyright (c) 2022 by the author alone
 *            https://gitlab.womolin.de/martin.verges/waterlevel
 * 
 * License: CC BY-NC-SA 4.0
 */

#ifndef TANKLEVEL_h
#define TANKLEVEL_h

#define NVS_WRITE_TOLERANCE_PA 250                 // Only write airpressure data to NVS if pressure difference is higher
#define MAX_DATA_POINTS 255                        // how many level data points to store (increased accuracy)
#include <Arduino.h>
#include <Preferences.h>
#include <HX711.h>

class TANKLEVEL
{
	private:
        // we cut of values due to sensor or adc noise
        float LOWER_END = 0.020;                   // value increase to start recording (tank is empty)
        float UPPER_END = 0.980;                   // value limit to cutoff data (tank is full)

        String NVS;                                // NVS Storage to write and read values

        struct config_t {
            bool setupDone = false;                // Configuration done or not yet initialized sensor
            double offset = 0.0;                   // Offset (tare) value of an unpressurized sensor reading
            int airPressureOnFilling = 0;          // AirPressure Value at the time when filling the tank to compensate readings
            int readings[101] = {0};               // pressure readings to map to percentage filling 0% - 100%
            uint32_t volumeMilliLiters = 0;        // Tank volume in liters
        } levelConfig;

        int lastMedian = 0;                        // The last reading median sensor value
        int airPressure = 0;                       // current air pressure in hPa
        bool tankWasEmpty = false;                 // True, when the tank is empty to store new airPressure when filling again

        struct state_t {
            int start = false;                     // Async start the setup
            int abort = false;                     // Async Abort current running setup
            int end = false;                       // Async End current running setup
            int valueCount = 0;                    // current number of entries in readings[]
            int minValue = 0.00;                   // lowest value to start recording
            int readings[MAX_DATA_POINTS] = {0};   // data readings from pressure sensor while running level setup
        } setupConfig;

        struct timeing_t {
            // Update Sensor data in loop()
            uint64_t lastSensorRead = 0;                 // last millis() from Sensor read
            const uint32_t sensorIntervalMs = 5000;      // Interval in ms to execute code

            // Execute Setup procedure in loop()
            uint64_t lastSetupRead = 0;                              // last millis() from Setup run
            const uint32_t setupIntervalMs = 15 * 60 * 1000 / 255;   // Interval in ms to execute code
        } timing;

        HX711 hx711;
        Preferences preferences;

        // Search through the setupConfig sensor readings and find the lower limit cutoff index
        int findStartCutoffIndex(int endIndex);

        // Search through the setupConfig sensor readings and find the upper limit cutoff index
        int findEndCutoffIndex();

        // Reset the setupConfig struct
        void resetSetupData();

        // Write current leveldata to non volatile storage
        bool writeToNVS();

        // Automatically enable the Air Pump if air pressure greatly increases or decreases
        bool automaticAirPump = true;

        // Enable the Air Pump if atmospheric pressure in hPa 
        uint16_t automatichAirPumpOnPressureDifferenceHPA = 10;

        // True as long as the pump is running
        bool airPumpEnabled = false;

        // Time when the Air Pump was started
        uint64_t airPumpStarttime = 0;

        // GPIO PIN that enables the Air Pump on HIGH
        gpio_num_t airPumpPIN = GPIO_NUM_NC;

        // Runtime of the Air Pump in milliseconds
        uint64_t airPumpDurationMS = 5000;

        // Set the level variable to 0-100 according to the current state of lastMedian
        // You need to call getCalulcatedMedianReading() before calculateLevel() to update lastMedian
        uint8_t calculateLevel();

        // The current level set by calculateLevel()
        uint8_t level = 0;

	public:
        // Get the current level calculcated and updated in loop()
        uint8_t getLevel() { return level; }

        // get Last Median reading value updated in loop()
        int getLastMedian() { return lastMedian; }

        // The last sensor raw reading
        double lastRawReading = 0.0;

        // Configure the AirPump GPIO
        void setAirPumpPIN(gpio_num_t gpio);

        // Set a new duration for the Air Pump runtime
        void setAirPumpDuration(uint64_t d) { airPumpDurationMS = d; }

        // Start/Activate the Air Pump
        void activateAirPump();

        // Stop/Deactivate the Air Pump
        void deactivateAirPump();

        // Enable/Disable automatic repressurization
        void setAutomaticAirPump(bool enabled) { automaticAirPump = enabled; }

        // Auto pump air pressure threshold
        void setAirPressureThreshold(uint16_t x) { automatichAirPumpOnPressureDifferenceHPA = x; }

        // Get current air pump auto start threshold
        uint16_t getAirPressureThreshold() { return automatichAirPumpOnPressureDifferenceHPA; }

        // Set tank volume in milli Liters
        bool setMaxVolume(uint32_t tankvolume, String unit);
        
        // Get the max water tank volume
        uint32_t getMaxVolume() { return levelConfig.volumeMilliLiters; }

        // Get the current water tank volume in milliliters
        uint32_t getCurrentVolume() { return levelConfig.volumeMilliLiters * level / 100; }

        // call loop
        void loop();
    
		TANKLEVEL(uint8_t dout, uint8_t pd_sck, gpio_num_t airPumpPIN);

        // Initialize the Webserver
		void begin(String ns = "tanksensor");

        // Configure uper and lower cutoff values for the setup (drop bad readings)
        void setCutoffLimits(float lower_end, float upper_end);

        // Read Median(10) value from sensor and return it unmodifed
        double getSensorRawMedianReading(bool cached = false);

        // Read Median(10) value from sensor and optimize/modify directly
        int getCalulcatedMedianReading(bool cached = false);

        // Calculate current level in percent. Requires valid level setup.
        // int getCalculatedPercentage(bool cached = false);

        // Get the configured level for a percentage value
        int getLevelData(int perc);

        // Check if level setup was done
        bool isConfigured();

        // Check if a level setup is currently running
        bool isSetupRunning();

        // Create a level db from lower and upper reading (only for tanks with linear form)
        bool setupFrom2Values(int lower, int upper);

        // Write a single level data entry to NVS, i=0-100%, 255 value 0 or 1 for levelsetup done
        bool writeSingleEntrytoNVS(uint8_t i, int value);

        // Write a new offset into NVS
        bool updateOffsetNVS();

        // Write the AirPressure for compenation into NVS
        bool updateAirPressureNVS(uint32_t newPressure);

        // Start a new level setup
        bool beginLevelSetup();

        // Record a new reading in level setup mode
        int runLevelSetup();

        // End the level setup and store data to NVS
        bool endLevelSetup();

        // Request to start a new level Setup
        void setStartAsync() { setupConfig.start = true; };

        // Request an end to the current running level setup
        void setEndAsync() { setupConfig.end = true; };

        // Request an abort of the current running level setup
        void setAbortAsync() { setupConfig.abort = true; };

        // Abort the current running level setup without storing it to NVS
        bool abortLevelSetup();

        // helper to get ESP32 runtime
        uint64_t runtime();

        // Set a new sensor offset from current sensor reading
        void setSensorOffset(double newOffset = 0.0, bool updateNVS = true);
        
        // Get the current sensor offset value
        double getSensorOffset() { return levelConfig.offset; };

        // Update the current evironmental pressure in hPa to compensate sensor reading
        void setAirPressure(int32_t hPa);

        // Get the current configured AirPressure
        int getAirPressure() { return airPressure; }
};

#endif /* TANKLEVEL_h */
