#include <Wire.h>
#include <HardwareSerial.h>

// CO2 sensor
#include "CO2Sensor.h"

// SO2 and H2S sensor
#include <MQUnifiedsensor.h>

// MICS Sensor
#include <MICS_4514.h>

// Define pins
#define MG811_PIN 12
#define MQ136_PIN 14 
#define MQ4_PIN 27
#define MICS_NOX_PIN 26   // MICS OX/NOX sensor (NO2)
#define MICS_RED_PIN 25  // MICS RED sensor (CO, hydrocarbons)
#define MICS_PRE_PIN 33  // Heater power control
#define BAUDRATE 9600

//Definitions MQ Sensors
#define Board "ESP32"
#define Voltage_Resolution 3.3
#define ADC_Bit_Resolution 12 // For ESP32

//Define Variables
float calcR0; //for MQ Sensors
float gasdata; //for MICS Sensor
float RatioMQ136CleanAir = 3.6; //RS / R0 = 3.6 ppm  
float RatioMQ4CleanAir = 4.4; //RS / R0 = 60 ppm 
float CO2_inertia = 0.99;
int CO2_tries = 100;
float MQ136_H2S_A = 36.737;
float MQ136_H2S_B = -3.536;
float MQ136_SO2_A = 503.34;
float MQ136_SO2_B = -3.774;
float MQ4_CH4_A = 1012.7;
float MQ4_CH4_B = -2.786;

//data
float co2 = 0.0, so2 = 0.0, h2s = 0.0, ch4 = 0.0, no2 = 0.0, co = 0.0;

//Declare Sensor
CO2Sensor co2Sensor(MG811_PIN, CO2_inertia, CO2_tries);
MICS_4514 MICS_4514(MICS_RED_PIN, MICS_NOX_PIN, MICS_PRE_PIN);
MQUnifiedsensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ4_PIN, "MQ-4");
MQUnifiedsensor MQ136(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ136_PIN, "MQ-136");

void setup() {
    //Init serial port
    Serial.begin(BAUDRATE);

    //MG-811 Setup
    co2Sensor.calibrate();

    //MICS-4514 Setup
    MICS_4514.setR0();

    //MQ-136 Setup
    MQ136.init(); 
    MQ136.setRegressionMethod(1); //_PPM =  a*ratio^b
    Serial.print("MQ-136 Calibrating...");
    calcR0 = 0;
    for(int i = 1; i<=10; i ++){
        MQ136.update(); 
        calcR0 += MQ136.calibrate(RatioMQ136CleanAir);
    }
    MQ136.setR0(calcR0/10);
    if(isinf(calcR0) || (calcR0 == 0)) {
        Serial.println("Invalid");
    } else {
        Serial.println("  done!.");
    }

    //MQ-4 Setup
    MQ4.init(); 
    MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
    Serial.print("MQ-4 Calibrating...");
    calcR0 = 0;
    for(int i = 1; i<=10; i ++){
        MQ4.update();
        calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    }
    MQ4.setR0(calcR0/10);
    if(isinf(calcR0) || (calcR0 == 0)) {
        Serial.println("Invalid");
    } else {
        Serial.println("  done!.");
    }

    //data_title
    Serial.println(F("co2 | so2 | h2s | ch4 | no2 | co"));
}

void loop() {
    // Read data from sensors
    co2 = readCO2();
    so2 = readSO2(MQ136_SO2_A, MQ136_SO2_B);
    h2s = readH2S(MQ136_H2S_A, MQ136_H2S_B);
    ch4 = readCH4(MQ4_CH4_A, MQ4_CH4_B);
    no2 = readNO2();
    co = readCO();

    // Log data
    logData();
    delay(1000);
}

float readCO2() {
    return (co2Sensor.read());
}   

float readH2S(float A, float B) {
    MQ136.update(); 
    MQ136.setA(A); MQ136.setB(B); 
    return (MQ136.readSensor()); 
}

float readSO2(float A, float B) {
    MQ136.update(); 
    MQ136.setA(A); MQ136.setB(B); 
    return (MQ136.readSensor()); 
}

float readCH4(float A, float B) {
    MQ4.update(); 
    MQ4.setA(A); MQ4.setB(B); 
    return (MQ4.readSensor()); 
}

float readNO2() {
    return MICS_4514.getNitrogenDioxide();
}

float readCO() {
    return MICS_4514.getCarbonMonoxide();
}

void logData() {
    Serial.print(co2); Serial.print(" | ");
    Serial.print(so2); Serial.print(" | ");
    Serial.print(h2s); Serial.print(" | ");
    Serial.print(ch4); Serial.print(" | ");
    Serial.print(no2); Serial.print(" | ");
    Serial.print(co); Serial.println();
}
