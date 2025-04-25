#include "MICS_4514.h"

#define PIN_RED A0
#define PIN_NOX A1
#define PIN_PRE 13

MICS_4514 GasSensor(PIN_RED, PIN_NOX, PIN_PRE);
// Create Class for MICS-4514 Sensor

void setup() {
  Serial.begin(9600);
  // Setup Serial Communication

  GasSensor.setWarmupTime(10000);    // 10000 ms = 10 sec
  // Set MICS-4514 warmup time (unit: ms)
  // Default warmup time is 3 min

  GasSensor.warmupStart();
  // Start MICS-4514 warmup

  while(!GasSensor.sensorReady()) {
    Serial.println("Preheating Gas Sensor...");
    delay(2000);
  }
  // Wait until MICS-4514 warmup done
  Serial.println("Gas Sensor Ready");

  GasSensor.setR0();
  // After MICS-4514 warmup process done, we need to calculate R0 (R0: Sensor Initial Resistance)
  // The R0 must be measured in clean air


  float gas;
  
  float gas = GasSensor.getCarbonMonoxide();
  // get CO measurement value (unit: ppm)

  float gas = GasSensor.getNitrogenDioxide();
  // get NO2 measurement value (unit: ppm)

  float gas = GasSensor.getEthanol();
  // get C2H5OH measurement value (unit: ppm)

  float gas = GasSensor.getHydrogen();
  // get H2 measurement value (unit: ppm)
  
  float gas = GasSensor.getAmmonia();
  // get NH3 measurement value (unit: ppm)

  float gas = GasSensor.getMethane();
  // get CH4 measurement value (unit: ppm)

  if (GasSensor.getHeatingState()) {
    // do something when heater is on
  } else {
    // do something when heater is off
  }

  GasSensor.setHeatingState(HIGH);
  // turn on heater

  // GasSensor.setHeatingState(LOW);
  // turn off heater
}

float gas_co;

void loop() {

  if (GasSensor.sensorReady()) {
    gas_co = GasSensor.getCarbonMonoxide();
    // If the sensor value is out of range, the value is -1 (range is described in the datasheet)

    if (gas_co != -1) {
      Serial.print("CO: ");
      Serial.println(gas_co);
    }
    else {
      Serial.println("Value is not valid");
    }
  }
  delay(1000);
}
