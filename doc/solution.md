## ESP32 ADC: Achieving Accurate and Consistent AnalogRead Measurements

You are correct: the ESP32’s ADC is **not highly accurate out-of-the-box**, and raw readings are often inconsistent, non-linear, and affected by several hardware and software factors. However, with proper techniques, you can significantly improve reliability for voltage measurement and sensor applications.

Below is a **step-by-step guide** and a **tested code example** to help you get the most accurate readings possible on the ESP32.

---

## 1. **Understand ESP32 ADC Limitations**

- **Non-linearity:** ESP32 ADC is not perfectly linear. Raw readings do not map 0-4095 exactly to 0-3.3V.
- **Vref Variation:** The reference voltage (Vref) can vary between chips, usually 1.0V–1.2V, not 3.3V.
- **Channel Crosstalk:** Switching between channels too quickly causes “ghosting” (residual readings from the previous pin).
- **WiFi Interference:** ADC2 channels cannot be used reliably while WiFi is active. Use **ADC1** pins (GPIO32-39).
- **Calibration:** Factory calibration is available on ESP32, but you must use the ESP-IDF or Arduino’s `analogReadMilliVolts()` (if available).

---

## 2. **Hardware Recommendations**

- Use **ADC1** pins for all analog measurements (GPIO32, 33, 34, 35, 36, 39).
- Add a **100nF ceramic capacitor** between each analog pin and GND (close to the pin) to filter noise.
- Make sure your voltage dividers are accurate (use 1% resistors).
- Ensure your input never exceeds 3.3V (damage risk).

---

## 3. **Software Techniques for Reliable Readings**

### a. **Discard Initial Readings**
The first few reads after switching channels or power-up are unreliable.

### b. **Add Settling Delay**
Wait a few microseconds after switching channels.

### c. **Average Multiple Samples**
Take 16–32 samples and average them for each reading.

### d. **Use Correct Attenuation**
Set attenuation to match your input voltage range.

### e. **Calibration**
If possible, use `analogReadMilliVolts()` or calibrate manually using known voltages.

---

## 4. **Example: Accurate Voltage Reading on ESP32**

Below is a **standalone example** for reading voltages on pins 32, 33, 34, and 35, with all the best practices included.

```cpp
#include 

// Pin assignments (ADC1 only)
#define PIN1 34  // 3.3V direct
#define PIN2 35  // 2.473V (via divider)
#define PIN3 32  // 1.484V (via divider)
#define PIN4 33  // Calculate this based on your divider

// Number of samples for averaging
#define NUM_SAMPLES 32

// Helper: Average analogRead with settling and discard
uint16_t readStableADC(uint8_t pin) {
  uint32_t sum = 0;
  // Discard first read after switching channel
  analogRead(pin);
  delayMicroseconds(50);
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(10);
  }
  return sum / NUM_SAMPLES;
}

// Helper: Convert ADC reading to voltage (manual calibration)
float adcToVoltage(uint16_t adc, float vref = 3.3, int bits = 12) {
  return ((float)adc / (float)((1 << bits) - 1)) * vref;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set attenuation for full 3.3V range
  analogSetAttenuation(ADC_11db); // Allows up to ~3.3V
  analogSetWidth(12);             // 12-bit resolution

  Serial.println("ESP32 ADC Test - Accurate Readings");

  // Warm up ADC by discarding some readings
  for (int i = 0; i < 3; i++) {
    readStableADC(PIN1);
    readStableADC(PIN2);
    readStableADC(PIN3);
    readStableADC(PIN4);
  }
}

void loop() {
  uint16_t adc1 = readStableADC(PIN1);
  uint16_t adc2 = readStableADC(PIN2);
  uint16_t adc3 = readStableADC(PIN3);
  uint16_t adc4 = readStableADC(PIN4);

  float v1 = adcToVoltage(adc1);
  float v2 = adcToVoltage(adc2);
  float v3 = adcToVoltage(adc3);
  float v4 = adcToVoltage(adc4);

  Serial.printf("PIN1 (%.2fV): ADC=%d, V=%.3fV\n", 3.3, adc1, v1);
  Serial.printf("PIN2 (%.2fV): ADC=%d, V=%.3fV\n", 2.473, adc2, v2);
  Serial.printf("PIN3 (%.2fV): ADC=%d, V=%.3fV\n", 1.484, adc3, v3);
  Serial.printf("PIN4 (??V):    ADC=%d, V=%.3fV\n", adc4, v4);

  delay(2000);
}
```

---

## 5. **Calibration & Correction**

### a. **Manual Calibration**
If your readings are consistently off, calibrate:
- Measure the actual voltage with a multimeter.
- Note the ADC value.
- Adjust your `vref` in `adcToVoltage()` until calculated voltage matches the multimeter.

### b. **Use analogReadMilliVolts() (if available)**
Some ESP32 Arduino cores support:
```cpp
uint32_t mv = analogReadMilliVolts(pin);
Serial.printf("Pin %d: %d mV\n", pin, mv);
```
This uses the factory-calibrated Vref.

---

## 6. **Summary Table: Your Example**

| Pin  | Expected Voltage | Expected ADC | Typical Raw ADC | Notes                             |
|------|------------------|--------------|-----------------|-----------------------------------|
| 34   | 3.3V             | 4095         | ~4000-4095      | Slightly less if Vref < 3.3V      |
| 35   | 2.473V           | ~3071        | ~2800-3000      | Nonlinearity, Vref, attenuation   |
| 32   | 1.484V           | ~1840        | ~1700-1800      |                                   |
| 33   | (calc)           | (calc)       | (measure)       |                                   |

---

## 7. **Final Tips**

- Always use **ADC1** pins (32, 33, 34, 35, 36, 39) for analog reads if WiFi is used.
- Set attenuation to `ADC_11db` for full voltage range.
- Use averaging and discard first reads.
- Calibrate your readings with a known reference voltage.
- For best accuracy, use `analogReadMilliVolts()` if your core supports it.

---

## 8. **References**

- [ESP32 ADC Arduino Documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html)
- [Espressif ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP32 ADC Non-linearity and Calibration](https://github.com/espressif/esp-idf/issues/1643)

---

**Try the code above and compare the Serial Monitor output to your multimeter readings.  
If you need further calibration or have more questions, just ask!**

Citations:
[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/60673637/fb155a4b-f340-4463-a36d-5b72ab03d87e/paste.txt
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/60673637/fb155a4b-f340-4463-a36d-5b72ab03d87e/paste.txt

---
Answer from Perplexity: https://www.perplexity.ai/search/in-esp32-dev-when-run-a-progra-cP1ar_n1SByyw3JSt.B8dQ?utm_source=copy_output