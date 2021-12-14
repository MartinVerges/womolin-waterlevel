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
            int valueCount = 0;                    // current number of entries in readings[]
            int minValue = 0.00;                   // lowest value to start recording
            int lastread = 0.00;                   // last redading while in setup  
            int readings[MAX_DATA_POINTS] = {0};   // data readings from pressure sensor while running level setup
        } setupConfig;

        // Search through the setupConfig sensor readings and find the lower limit cutoff index
        int findStartCutoffIndex(int endIndex);

        // Search through the setupConfig sensor readings and find the upper limit cutoff index
        int findEndCutoffIndex();

        void printData(int* readings, size_t count);

	public:
		TANKLEVEL();

		virtual ~TANKLEVEL();

		void begin(uint8_t dout, uint8_t pd_sck, String nvs);
        void setLimits(float lower_end, float upper_end);
        int getMedian();
        int getPercentage(bool cached = false);
        bool isConfigured();
        bool isSetupRunning();
        void endLevelSetup();

        void levelSetup();

        // Get the current tank level percentage
        int getPercentage(float val);

        // Print out the current level database
        void printData();
};

#endif /* TANKLEVEL_h */
