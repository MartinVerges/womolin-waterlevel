/**
 *
 * Tanklevel Pressure Sensor
 * https://github.com/MartinVerges/rv-smart-tanksensor
 *
 * (c) 2021 Martin Verges
 *
**/

#ifndef TANKLEVEL_h
#include "Arduino.h"
#define MAX_DATA_POINTS 255                        // how many level data points to store (increased accuracy)

class TANKLEVEL
{
	private:
        float LOWER_END = 0.010;                   // value increase to start recording (tank is empty)
        float UPPER_END = 0.990;                   // value limit to cutoff data (tank is full)
        String NVS_NAMESPACE = "tanksensor";       // NVS Storage to write and read values

        struct config_t {
            bool setupDone = false;                // Configuration done or not yet initialized sensor
            int readings[100] = {0};               // pressure readings to map to percentage filling
        } levelConfig;
        int currentState = 0;                      // last reading in percent

        struct state_t {
            int abort = false;                     // Abort current running setup (due to ASYNC issues as var)
            int valueCount = 0;                    // current number of entries in readings[]
            int minValue = 0.00;                   // lowest value to start recording
            int lastread = 0.00;                   // last redading while in setup  
            int readings[MAX_DATA_POINTS] = {0};   // data readings from pressure sensor while running level setup
        } setupConfig;

        // Search through the setupConfig sensor readings and find the lower limit cutoff index
        int findStartCutoffIndex(int endIndex);

        // Search through the setupConfig sensor readings and find the upper limit cutoff index
        int findEndCutoffIndex();

        // Print out the reading array to Serial (for debugging) 
        void printData(int* readings, size_t count);

	public:
		TANKLEVEL();
		virtual ~TANKLEVEL();

        // Initialize the Webserver
		void begin(uint8_t dout, uint8_t pd_sck, String nvs);

        // Configure uper and lower cutoff values for the setup (drop bad readings)
        void setLimits(float lower_end, float upper_end);

        // Read Median(10) raw value from sensor
        int getMedian();

        // Calculate current level in percent. Requires valid level setup.
        int getPercentage(bool cached = false);

        // Check if level setup was done
        bool isConfigured();

        // Check if a level setup is currently running
        bool isSetupRunning();

        // Start a new level setup
        bool beginLevelSetup();

        // Record a new reading in level setup mode
        int runLevelSetup();

        // End the level setup and store data to NVS
        bool endLevelSetup();

        // Request an abort of the current running level setup
        void setAbortAsync();

        // Abort the current running level setup without storing it to NVS
        bool abortLevelSetup();

        // Print out the current level configuration database
        void printData();
};

#endif /* TANKLEVEL_h */
