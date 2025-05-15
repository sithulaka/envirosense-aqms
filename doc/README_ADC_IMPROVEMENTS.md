# ESP32 ADC Improvements

This document explains the modifications made to address common ESP32 ADC issues and improve analog reading reliability. ESP32 microcontrollers often exhibit unstable or incorrect initial ADC readings after power-on due to several hardware and initialization characteristics.

## Issues Addressed

1. **Non-linearity:** ESP32 ADC is not perfectly linear across its range
2. **Initial Readings:** First few ADC readings after power-on are often incorrect
3. **Channel Switching:** Switching between ADC channels can cause "ghosting" effects
4. **Stabilization Time:** ADC subsystem requires time to stabilize after power-up
5. **Vref Variations:** Reference voltage stabilization issues affect measurement accuracy

## Implemented Solutions

### 1. Stable ADC Reading Function (`readStableADC`)

Added in `esp_main.cpp`, this function:
- Discards initial readings
- Adds settling time delays
- Averages multiple samples for noise reduction

```cpp
uint16_t readStableADC(uint8_t pin) {
  uint32_t sum = 0;
  
  // Discard first readings (they're often incorrect on ESP32)
  analogRead(pin);
  analogRead(pin);
  delayMicroseconds(50);
  
  // Average multiple samples
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(10); // Small delay between readings
  }
  
  return sum / NUM_SAMPLES;
}
```

### 2. ADC Configuration

Added proper ADC configuration in setup():
- Set ADC resolution to 12-bit (0-4095)
- Configure attenuation for full voltage range
- Perform initial warm-up readings

```cpp
analogSetWidth(12);               // 12-bit resolution (0-4095)
analogSetAttenuation(ADC_11db);   // Full voltage range (0-3.3V)
```

### 3. Sensor Library Extensions

Created sensor extensions with improved ADC handling:

- **MQSensorWrapper**: Extends MQUnifiedsensor to override the update() method with stable ADC readings
- **MICS_4514_Extended**: Creates an extended version of MICS_4514 with stable ADC readings and exposed methods
- **CO2SensorWrapper**: Extends CO2Sensor with stable ADC readings while maintaining local copies of variables

Each implementation was tailored to work with the original library's design and access patterns.

### 4. Enhanced Warmup Procedure

Improved the Warmup function to:
- Add an initial ADC stabilization phase
- Continue ADC readings during warmup to maintain stability
- Add a final stabilization phase at the end

```cpp
void Warmup(unsigned long warmupTime) {
    // Initial ADC stabilization
    // Wait for MICS sensor warmup
    // Continue taking ADC readings during warmup
    // Final stabilization period
}
```

## File Organization

The extension classes have been organized in the following locations:

1. **CO2SensorWrapper.h**: Located in `libraries/CO2Sensor-master/src/`
   - Extends the CO2Sensor base class
   - Maintains local copies of variables to avoid private member access issues

2. **MQSensorWrapper.h**: Located in `libraries/MQSensorsLib-master/src/`
   - Extends the MQUnifiedsensor class
   - Uses the public externalADCUpdate() method to update sensor values

3. **MICS_4514_Extended.h**: Located in `libraries/MICS_4514_Arduino/`
   - Creates a new extended class that reimplements gas calculation methods
   - Uses the stable ADC reading function for accurate measurements
   - Maintains its own copies of calibration values (_r0_red_ext, _r0_ox_ext) since it can't access private members
   - Overrides setR0() to update both the parent's values and its own local copies

## Why These Changes Matter

ESP32's ADC behavior during initialization causes several issues that can significantly impact sensor accuracy:

1. **Sampling Circuit Stabilization**: Initial readings are incorrect until internal sampling circuits reach a stable state
2. **Channel Switching Delays**: Switching between ADC pins without delays causes reading contamination
3. **Calibration Processes**: Factory calibration parameters need time to load and apply
4. **Vref Stabilization**: Reference voltage needs time to stabilize

These issues particularly affect applications like air quality monitoring where accurate sensor readings are essential from the start.

## Technical Implementation

The implementation maintains compatibility with the existing codebase using two approaches:

1. **Wrapper Classes** (for CO2Sensor and MQUnifiedsensor):
   - Extend the original classes through inheritance
   - Override key methods to use stable ADC readings
   - Maintain local copies of private variables as needed

2. **Extended Classes** (for MICS_4514):
   - Create extended versions of the original classes
   - Reimplement calculation methods using stable ADC readings
   - Provide public access to functions that were previously private

## Compilation Notes and Troubleshooting

When implementing the sensor extensions, several compilation issues were addressed:

1. **Private Member Access**: 
   - For CO2SensorWrapper: Created local copies of variables (_pin, _myInertia, _myTries)
   - For MQSensorWrapper: Used externalADCUpdate() method instead of directly accessing private variables
   - For MICS_4514: Created extended class with reimplementation of gas calculation methods

2. **Library Design Considerations**:
   - MICS_4514 doesn't have separate calculate methods, so we reimplemented the full calculation logic
   - CO2Sensor doesn't have getter methods for inertia and tries, so we maintain local copies
   - MQUnifiedsensor has a convenient externalADCUpdate() we can use

3. **Forward Declarations**: Each wrapper/extension file includes a forward declaration of the `readStableADC()` function to ensure it's visible.

If you encounter additional compilation issues:
- Check for changes in the structure of the libraries
- Make sure the extended classes maintain compatibility with the original APIs
- Verify the sensor initialization and configuration remains compatible

## Usage Notes

The enhanced system maintains the same API while improving reading quality:
1. Properly initializes the ADC
2. Discards unreliable initial readings
3. Uses multiple samples with averaging
4. Properly handles transitions between different ADC channels

These improvements result in more accurate and consistent readings, especially immediately after power-on or reset. 