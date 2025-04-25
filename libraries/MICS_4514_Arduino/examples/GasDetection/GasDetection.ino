/*
  Gas Detection Example for MICS-4514 Sensor
  
  This example demonstrates how to use the MICS-4514 gas sensor to detect
  various gases using the updated library.
  
  The circuit:
  * Connect the RED/NOX analog output of the MICS-4514 to analog pins
  * Connect the PRE pin to a digital pin
  
  Created by: [Your Name]
  Date: [Current Date]
*/

#include <MICS_4514.h>

// Pin definitions
#define PIN_RED A0  // RED (CO) sensor connected to A0
#define PIN_NOX A1  // NOX (NO2) sensor connected to A1 
#define PIN_PRE 8   // PRE (heater) pin connected to D8

// Create MICS_4514 instance
MICS_4514 mics(PIN_RED, PIN_NOX, PIN_PRE);

void setup() {
  Serial.begin(9600);
  Serial.println("MICS-4514 Gas Sensor Test");
  
  // Set warmup time to 3 minutes
  mics.setWarmupTime(3 * 60 * 1000);
  
  Serial.println("Starting sensor warmup...");
  mics.warmupStart();
}

void loop() {
  if (mics.sensorReady()) {
    if (!mics.isR0Calibrated()) {
      Serial.println("Setting R0 baseline values...");
      mics.setR0();
      delay(1000);
      return;
    }
    
    // Read gas concentrations
    Serial.println("Gas Measurements:");
    
    // Carbon Monoxide (CO)
    float co = mics.getCarbonMonoxide();
    Serial.print("CO: ");
    Serial.print(co);
    Serial.println(" ppm");
    
    // Nitrogen Dioxide (NO2)
    float no2 = mics.getNitrogenDioxide();
    Serial.print("NO2: ");
    Serial.print(no2);
    Serial.println(" ppm");
    
    // Ethanol
    float ethanol = mics.getEthanol();
    Serial.print("Ethanol: ");
    Serial.print(ethanol);
    Serial.println(" ppm");
    
    // Hydrogen
    float hydrogen = mics.getHydrogen();
    Serial.print("H2: ");
    Serial.print(hydrogen);
    Serial.println(" ppm");
    
    // Ammonia
    float ammonia = mics.getAmmonia();
    Serial.print("NH3: ");
    Serial.print(ammonia);
    Serial.println(" ppm");
    
    // Methane
    float methane = mics.getMethane();
    Serial.print("CH4: ");
    Serial.print(methane);
    Serial.println(" ppm");
    
    // Check for specific gas presence
    Serial.println("\nGas Detection:");
    
    if (mics.existGas(GAS_CO)) {
      Serial.println("CO detected!");
    }
    
    if (mics.existGas(GAS_NO2)) {
      Serial.println("NO2 detected!");
    }
    
    if (mics.existGas(GAS_NH3)) {
      Serial.println("NH3 detected!");
    }
    
    if (mics.existGas(GAS_C2H5OH)) {
      Serial.println("Ethanol detected!");
    }
    
    if (mics.existGas(GAS_H2)) {
      Serial.println("H2 detected!");
    }
    
    if (mics.existGas(GAS_CH4)) {
      Serial.println("Methane detected!");
    }
    
    if (mics.existGas(GAS_C3H8)) {
      Serial.println("Propane detected!");
    }
    
    if (mics.existGas(GAS_C4H10)) {
      Serial.println("Butane detected!");
    }
    
    delay(5000); // Wait 5 seconds between readings
  } else {
    // Display warmup progress
    unsigned long elapsed = millis() - mics.getWarmupStartTime();
    unsigned long remaining = mics.getWarmupTime() - elapsed;
    
    Serial.print("Warming up... ");
    Serial.print(remaining / 1000);
    Serial.println(" seconds remaining");
    
    delay(1000); // Update every second
  }
} 