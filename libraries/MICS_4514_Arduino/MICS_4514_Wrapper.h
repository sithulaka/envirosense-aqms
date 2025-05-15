#ifndef _MICS_4514_WRAPPER_H_
#define _MICS_4514_WRAPPER_H_

/*
 * MICS-4514 Sensor Wrapper
 * 
 * This wrapper enhances the MICS_4514 library to use stable ADC readings on ESP32.
 * It overrides the getVoltNO2() and getVoltRed() methods to use readStableADC() 
 * instead of direct analogRead().
 * 
 * Benefits:
 * - Improves reading stability for gas concentration measurements
 * - Discards unreliable initial readings
 * - Implements proper delays between readings
 * - Maintains compatibility with the original MICS_4514 API
 */

#include <MICS_4514.h>

// This class extends MICS_4514 with improved ESP32 ADC reading
class MICS_4514_Wrapper : public MICS_4514 {
public:
    // Use the parent constructor
    MICS_4514_Wrapper(int pin_red, int pin_nox, int pin_pre = -1) 
        : MICS_4514(pin_red, pin_nox, pin_pre) {
    }

    // Override readings with stable ADC
    float getVoltNO2() {
        int adc_value = readStableADC(_pin_nox);
        if (adc_value == 0) return 0.0;          // Avoid division by zero
        if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
        float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
        return v_sensor;
    }

    float getVoltRed() {
        int adc_value = readStableADC(_pin_red);
        if (adc_value == 0) return 0.0;          // Avoid division by zero
        if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
        float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
        return v_sensor;
    }

private:
    // Forward declaration - this will use the global function
    uint16_t readStableADC(uint8_t pin);
    
    // Access private members from base class
    using MICS_4514::_pin_red;
    using MICS_4514::_pin_nox;
    using MICS_4514::MCU_ADC_MAX_VAL;
    using MICS_4514::V_MCU_VCC;
};

#endif // _MICS_4514_WRAPPER_H_ 