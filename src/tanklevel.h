/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2022 Martin Verges
 *
**/

#ifndef TANKLEVEL_h
#define TANKLEVEL_h

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

        String NVS = "tanksensor";                 // NVS Storage to write and read values

        struct config_t {
            bool setupDone = false;                // Configuration done or not yet initialized sensor
            int readings[101] = {0};               // pressure readings to map to percentage filling 0% - 100%
        } levelConfig;

        int lastMedian = 0;                        // last reading median value

        struct state_t {
            int start = false;                     // Async start the setup
            int abort = false;                     // Async Abort current running setup
            int end = false;                       // Async End current running setup
            int valueCount = 0;                    // current number of entries in readings[]
            int minValue = 0.00;                   // lowest value to start recording
            int readings[MAX_DATA_POINTS] = {0};   // data readings from pressure sensor while running level setup
        } setupConfig;

        HX711 hx711;
        Preferences preferences;

        // Search through the setupConfig sensor readings and find the lower limit cutoff index
        int findStartCutoffIndex(int endIndex);

        // Search through the setupConfig sensor readings and find the upper limit cutoff index
        int findEndCutoffIndex();

        // Print out the reading array to Serial (for debugging) 
        void printData(int* readings, size_t count);

        // Reset the setupConfig struct
        void resetSetupData();

        // Write current leveldata to non volatile storage
        bool writeToNVS();

	public:
		TANKLEVEL();
		virtual ~TANKLEVEL();

        // Initialize the Webserver
		void begin(uint8_t dout, uint8_t pd_sck, String nvs);

        // Configure uper and lower cutoff values for the setup (drop bad readings)
        void setLimits(float lower_end, float upper_end);

        // Read Median(10) raw value from sensor
        int getSensorMedianValue(bool cached = false);

        // Calculate current level in percent. Requires valid level setup.
        int getCalculatedPercentage(bool cached = false);

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

        // Start a new level setup
        bool beginLevelSetup();

        // Record a new reading in level setup mode
        int runLevelSetup();

        // End the level setup and store data to NVS
        bool endLevelSetup();

        // Request to start a new level Setup
        void setStartAsync();

        // Request an end to the current running level setup
        void setEndAsync();

        // Request an abort of the current running level setup
        void setAbortAsync();

        // Abort the current running level setup without storing it to NVS
        bool abortLevelSetup();

        // Print out the current level configuration database
        void printData();

        // helper to get ESP32 runtime
        uint64_t runtime();
};

#endif /* TANKLEVEL_h */
