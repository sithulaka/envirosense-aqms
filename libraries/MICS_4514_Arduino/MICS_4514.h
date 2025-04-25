/*
Author: jehongjeon27
version: 1.0
*/

#ifndef MICS_4514_H
#define MICS_4514_H

#include "Arduino.h"

#define V_MCU_VCC         5       // check your MCU ADC VREF!!
#define MCU_ADC_MAX_VAL  1023    // check your MCU ADC Resolution!!

#define V_SENSOR_VCC     5
#define R_LOAD_RED       750000   // check your module register value!!
#define R_LOAD_OX        2200   // check your module register value!!

class MICS_4514 {
  private:
    uint32_t _warmup_start_time = 0;
    uint32_t _warmup_time = 3*60*1000;    // default preheat time is 3 minutes
    bool _warmup_started = false;
    bool _warmup_finished = false;
    uint8_t _pin_red;
    uint8_t _pin_nox;
    uint8_t _pin_pre;
    float _r0_ox  = 0;
    float _r0_red = 0;
    bool _heating_status = false;

    float read_R_RED();
    float read_R_OX();

  public:
    MICS_4514(uint8_t pin_red, uint8_t pin_nox, uint8_t pin_pre);
    void setWarmupTime(uint32_t time);
    void warmupStart();
    bool sensorReady();
    void setR0();
    void setHeatingState(uint8_t on_off);
    bool getHeatingState();
    float getCarbonMonoxide();
    float getNitrogenDioxide();
    float getEthanol();
    float getHydrogen();
    float getAmmonia();
    float getMethane();
};

#endif
