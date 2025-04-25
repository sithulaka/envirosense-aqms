/*
  Author: jehongjeon27
  version: 1.1
*/

#include "MICS_4514.h"
#include "Arduino.h"

//Constructor
MICS_4514::MICS_4514(uint8_t pin_red, uint8_t pin_nox, uint8_t pin_pre) {

  _pin_red = pin_red;
  _pin_nox = pin_nox;
  _pin_pre = pin_pre;

  pinMode(_pin_red, INPUT);
  pinMode(_pin_nox, INPUT);
  pinMode(_pin_pre, OUTPUT);
  digitalWrite(_pin_pre, LOW);
}

void MICS_4514::setWarmupTime(uint32_t time) {
  _warmup_time = time;
}

void MICS_4514::warmupStart() {
  _warmup_started = true;
  _warmup_start_time = millis();
  digitalWrite(_pin_pre, HIGH);
  _heating_status = true;
}

bool MICS_4514::sensorReady() {
  if (_warmup_started) {
    if (_warmup_finished) {
      return true;
    } else {
      if (millis() - _warmup_start_time < _warmup_time) {
        return false;
      } else {
        _warmup_finished = true;
        return true;
      }
    }

  } else {
    return false;
  }
}

bool MICS_4514::getHeatingState() {
  return _heating_status;
}

void MICS_4514::setHeatingState(uint8_t on_off) {
  digitalWrite(_pin_pre, on_off);
  _heating_status = on_off ? true : false;
}

float MICS_4514::read_R_RED() {
  int adc_value = analogRead(_pin_red);
  if (adc_value == 0) return 0.0;          // Avoid division by zero
  if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
  float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
  if (v_sensor < 0.001) return 1e12;       // Avoid division by near-zero
  return R_LOAD_RED * (V_SENSOR_VCC - v_sensor) / v_sensor;
}

float MICS_4514::read_R_OX() {
  int adc_value = analogRead(_pin_nox);
  if (adc_value == 0) return 0.0;          // Avoid division by zero
  if (adc_value >= MCU_ADC_MAX_VAL) return 1e12; // Return a large value instead of INF
  float v_sensor = (float)adc_value / MCU_ADC_MAX_VAL * V_MCU_VCC;
  if (v_sensor < 0.001) return 1e12;       // Avoid division by near-zero
  return R_LOAD_OX * (V_SENSOR_VCC - v_sensor) / v_sensor;
}

void MICS_4514::setR0() {
  if (!sensorReady()) {
    return;
  }
  
  // Take multiple readings and average them for more stable R0
  float r_red_sum = 0;
  float r_ox_sum = 0;
  const int samples = 10;
  
  for (int i = 0; i < samples; i++) {
    r_red_sum += read_R_RED();
    r_ox_sum += read_R_OX();
    delay(50);
  }
  
  _r0_red = r_red_sum / samples;
  _r0_ox = r_ox_sum / samples;
}

float MICS_4514::getCarbonMonoxide() {
  if (!sensorReady() || _r0_red == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_RED() / _r0_red;
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

float MICS_4514::getEthanol() {
  if (!sensorReady() || _r0_red == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_RED() / _r0_red;
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

float MICS_4514::getMethane() {
  if (!sensorReady() || _r0_red == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_RED() / _r0_red;
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

float MICS_4514::getNitrogenDioxide() {
  if (!sensorReady() || _r0_ox == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_OX() / _r0_ox;
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

float MICS_4514::getHydrogen() {
  if (!sensorReady() || _r0_red == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_RED() / _r0_red;
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

float MICS_4514::getAmmonia() {
  if (!sensorReady() || _r0_red == 0) {
    return -1.0;
  }
  
  float rs_r0_ratio = read_R_RED() / _r0_red;
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




