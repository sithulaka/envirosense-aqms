#ifndef _MQ_SENSOR_WRAPPER_H_
#define _MQ_SENSOR_WRAPPER_H_

/*
 * MQ Sensor Wrapper
 * 
 * This wrapper enhances the MQUnifiedsensor library to use stable ADC readings on ESP32.
 * It overrides the update() method to use readStableADC() instead of direct analogRead().
 * 
 * Benefits:
 * - Discards unreliable initial readings
 * - Implements multi-sample averaging
 * - Adds proper delays between readings
 * - Maintains compatibility with the original MQUnifiedsensor API
 */

#include <MQUnifiedsensor.h>

// Forward declaration of the global stable ADC function
uint16_t readStableADC(uint8_t pin);

// This class extends MQUnifiedsensor with improved ESP32 ADC reading
class MQSensorWrapper : public MQUnifiedsensor {
private:
    int _sensorPin; // Store our own copy of the pin

public:
    // Constructor - passes everything to the parent class but stores the pin
    MQSensorWrapper(String Placa, float Voltage_Resolution, int ADC_Bit_Resolution, int pin, String type) 
        : MQUnifiedsensor(Placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type), _sensorPin(pin) {
    }

    // Override the update method to use our stable ADC reading
    void update() {
        // Use stable ADC reading instead of direct analogRead
        int adc_value = readStableADC(_sensorPin);
        
        // Convert ADC to voltage (similar to what getVoltage does internally)
        float voltage = (adc_value * getVoltResolution()) / ((pow(2, 12)) - 1); // Assuming 12-bit ADC on ESP32
        
        // Use external ADC update method which is public
        externalADCUpdate(voltage);
    }
};

#endif // _MQ_SENSOR_WRAPPER_H_ 