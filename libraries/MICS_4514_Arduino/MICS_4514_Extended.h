#ifndef MICS_4514_EXTENDED_H
#define MICS_4514_EXTENDED_H

#include "MICS_4514.h"

// Forward declaration of the global stable ADC function
uint16_t readStableADC(uint8_t pin);

// Extended version of MICS_4514 that exposes voltage reading as public methods
class MICS_4514_Extended : public MICS_4514 {
private:
    uint8_t _pin_red_ext;
    uint8_t _pin_nox_ext;
    
    // Store our own copies of R0 values since we can't access the private members
    float _r0_red_ext = 0;
    float _r0_ox_ext = 0;

public:
    MICS_4514_Extended(uint8_t pin_red, uint8_t pin_nox, uint8_t pin_pre = -1)
        : MICS_4514(pin_red, pin_nox, pin_pre), _pin_red_ext(pin_red), _pin_nox_ext(pin_nox) {
    }

    // Public method to get voltage from RED sensor using stable ADC
    float getVoltRed() {
        int adc_value = readStableADC(_pin_red_ext);
        if (adc_value == 0) return 0.0;          // Avoid division by zero
        if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
        float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
        return v_sensor;
    }

    // Public method to get voltage from NOX sensor using stable ADC
    float getVoltNOX() {
        int adc_value = readStableADC(_pin_nox_ext);
        if (adc_value == 0) return 0.0;          // Avoid division by zero
        if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
        float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
        return v_sensor;
    }

    // Get resistance from RED sensor using stable ADC
    float getResistanceRed() {
        float v_sensor = getVoltRed();
        if (v_sensor < 0.001) return 1e12;       // Avoid division by near-zero
        return R_LOAD_RED * (V_SENSOR_VCC - v_sensor) / v_sensor;
    }

    // Get resistance from NOX sensor using stable ADC
    float getResistanceNOX() {
        float v_sensor = getVoltNOX();
        if (v_sensor < 0.001) return 1e12;       // Avoid division by near-zero
        return R_LOAD_OX * (V_SENSOR_VCC - v_sensor) / v_sensor;
    }
    
    // Override setR0 to update our local copies
    void setR0() {
        // Call the parent method first
        MICS_4514::setR0();
        
        // Then store the results of multiple readings for our own use
        float r_red_sum = 0;
        float r_ox_sum = 0;
        const int samples = 10;
        
        for (int i = 0; i < samples; i++) {
            r_red_sum += getResistanceRed();
            r_ox_sum += getResistanceNOX();
            delay(50);
        }
        
        _r0_red_ext = r_red_sum / samples;
        _r0_ox_ext = r_ox_sum / samples;
    }

    // Override gas concentration calculation methods
    float getCarbonMonoxide() {
        if (!sensorReady() || _r0_red_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceRed() / _r0_red_ext;
        if (rs_r0_ratio > 0.425) {
            return 0.0;
        }
        float co = (0.425 - rs_r0_ratio) / 0.000405;
        if (co > 1000.0) {
            return 1000.0;
        }
        if (co < 1.0) {
            return 0.0;
        }
        return co;
    }

    float getEthanol() {
        if (!sensorReady() || _r0_red_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceRed() / _r0_red_ext;
        if (rs_r0_ratio > 0.306) {
            return 0.0;
        }
        float ethanol = (0.306 - rs_r0_ratio) / 0.00057;
        if (ethanol < 10.0) {
            return 0.0;
        }
        if (ethanol > 500.0) {
            return 500.0;
        }
        return ethanol;
    }

    float getNitrogenDioxide() {
        if (!sensorReady() || _r0_ox_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceNOX() / _r0_ox_ext;
        if (rs_r0_ratio < 1.1) {
            return 0.0;
        }
        float nitrogendioxide = (rs_r0_ratio - 0.045) / 6.13;
        if (nitrogendioxide < 0.1) {
            return 0.0;
        }
        if (nitrogendioxide > 10.0) {
            return 10.0;
        }
        return nitrogendioxide;
    }

    float getHydrogen() {
        if (!sensorReady() || _r0_red_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceRed() / _r0_red_ext;
        if (rs_r0_ratio > 0.279) {
            return 0.0;
        }
        float hydrogen = (0.279 - rs_r0_ratio) / 0.00026;
        if (hydrogen < 1.0) {
            return 0.0;
        }
        if (hydrogen > 1000.0) {
            return 1000.0;
        }
        return hydrogen;
    }

    float getAmmonia() {
        if (!sensorReady() || _r0_red_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceRed() / _r0_red_ext;
        if (rs_r0_ratio > 0.8) {
            return 0.0;
        }
        float ammonia = (0.8 - rs_r0_ratio) / 0.0015;
        if (ammonia < 1.0) {
            return 0.0;
        }
        if (ammonia > 500.0) {
            return 500.0;
        }
        return ammonia;
    }

    float getMethane() {
        if (!sensorReady() || _r0_red_ext == 0) {
            return -1.0;
        }
        
        float rs_r0_ratio = getResistanceRed() / _r0_red_ext;
        if (rs_r0_ratio > 0.786) {
            return 0.0;
        }
        float methane = (0.786 - rs_r0_ratio) / 0.000023;
        if (methane < 1000.0) {
            return 0.0;
        }
        if (methane > 25000.0) {
            return 25000.0;
        }
        return methane;
    }
};

#endif // MICS_4514_EXTENDED_H 