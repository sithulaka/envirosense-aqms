#ifndef _CO2_SENSOR_WRAPPER_H_
#define _CO2_SENSOR_WRAPPER_H_

/*
 * CO2 Sensor Wrapper
 * 
 * This wrapper enhances the CO2Sensor library to use stable ADC readings on ESP32.
 * It overrides the read() method to use readStableADC() instead of direct analogRead().
 * 
 * Benefits:
 * - Improves CO2 reading reliability and accuracy
 * - Discards unreliable initial readings
 * - Maintains the original inertia and multiple-try functionality
 * - Uses multiple samples with proper delays between readings
 * - Maintains compatibility with the original CO2Sensor API
 */

#include "CO2Sensor.h"

// Forward declaration of the global stable ADC function
uint16_t readStableADC(uint8_t pin);

// This class extends CO2Sensor with improved ESP32 ADC reading
class CO2SensorWrapper : public CO2Sensor {
private:
    int _pin; // Store our own copy of the pin
    float _myInertia; // Store our own copy of inertia
    int _myTries; // Store our own copy of tries

public:
    // Use the parent constructor but store the pin
    CO2SensorWrapper(int pin, float inertia = 0.99, int tries = 5) 
        : CO2Sensor(pin, inertia, tries), _pin(pin), _myInertia(inertia), _myTries(tries) {
    }
    
    // Override setInertia to update our local copy too
    void setInertia(float inertia) {
        _myInertia = inertia;
        CO2Sensor::setInertia(inertia);
    }
    
    // Override setTries to update our local copy too
    void setTries(int tries) {
        _myTries = tries;
        CO2Sensor::setTries(tries);
    }

    // Override the read method to use stable ADC
    float read() {
        // Instead of direct analogRead, use the stable version
        int value = readStableADC(_pin);
        
        // Apply the inertia filter as in the original CO2Sensor
        // But using our own implementation since we can't access private members
        static bool firstReading = true;
        static float lastValue = 0;
        
        if (_myTries == 1) {
            return value;
        }

        // Multiple tries with inertia
        if (firstReading) {
            lastValue = value;
            firstReading = false;
            return value;
        }

        float currentValue = 0;
        for (int i = 0; i < _myTries; i++) {
            // Use stable reading for each try
            currentValue += readStableADC(_pin);
        }
        currentValue /= _myTries;

        // Apply inertia (low-pass filter)
        float result = lastValue * _myInertia + currentValue * (1.0 - _myInertia);
        lastValue = result;
        
        return result;
    }
};

#endif // _CO2_SENSOR_WRAPPER_H_ 