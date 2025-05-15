#ifndef _MQ_SENSOR_WRAPPER_H_
#define _MQ_SENSOR_WRAPPER_H_

/*
 * MQ Sensor Wrapper
 * 
 * This wrapper enhances the MQUnifiedsensor library to use stable ADC readings on ESP32.
 * It overrides the getVoltage() method to use readStableADC() instead of direct analogRead().
 * 
 * Benefits:
 * - Discards unreliable initial readings
 * - Implements multi-sample averaging
 * - Adds proper delays between readings
 * - Maintains compatibility with the original MQUnifiedsensor API
 */

#include <MQUnifiedsensor.h>

// This class extends MQUnifiedsensor with improved ESP32 ADC reading
class MQSensorWrapper : public MQUnifiedsensor {
public:
    // Constructor - just passes everything to the parent class
    MQSensorWrapper(String Placa, float Voltage_Resolution, int ADC_Bit_Resolution, int pin, String type) 
        : MQUnifiedsensor(Placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type) {
    }

    // Override the getVoltage method to use our stable reading function
    float getVoltage(bool read = true, bool injected = false, int value = 0) override {
        float voltage;
        if(read) {
            // Use our readStableADC function instead of direct analogRead
            uint16_t adc = readStableADC(this->_pin);
            voltage = adc * _VOLT_RESOLUTION / ((pow(2, _ADC_Bit_Resolution)) - 1);
        }
        else if(!injected) {
            voltage = _sensor_volt;
        }
        else {
            voltage = (value) * _VOLT_RESOLUTION / ((pow(2, _ADC_Bit_Resolution)) - 1);
            _sensor_volt = voltage; // to work on testing
        }
        return voltage;
    }

private:
    // We need access to the private _pin, etc.
    using MQUnifiedsensor::_pin;
    using MQUnifiedsensor::_sensor_volt;
    using MQUnifiedsensor::_VOLT_RESOLUTION;
    using MQUnifiedsensor::_ADC_Bit_Resolution;

    // Forward declaration - this will use the global function
    uint16_t readStableADC(uint8_t pin);
};

#endif // _MQ_SENSOR_WRAPPER_H_ 