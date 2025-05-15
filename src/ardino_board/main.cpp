#include <Wire.h>
#include <SoftwareSerial.h>

/*
 * EnviroSense AQMS - Arduino version with improvements
 * 
 * This code includes several improvements:
 * 1. Improved sensor handling
 * 2. Enhanced GPS data processing
 * 3. Better warmup sequences
 */

// SD card
#include <SD.h>

// GPS sensor
#include <TinyGPS++.h>

// CO2 sensor
#include "CO2Sensor.h"

// SO2 and H2S sensor
#include <MQUnifiedsensor.h>

// PM sensor
#include <SDS011.h>

// ENS160 sensor for eCO2
#include <DFRobot_ENS160.h>

// MICS Sensor
#include <MICS_4514.h>

// Define pins
#define MG811_PIN A0
#define MQ136_PIN A1
#define MQ4_PIN A2
#define MICS_NOX_PIN A6   // MICS OX/NOX sensor (NO2)
#define MICS_RED_PIN A7   // MICS RED sensor (CO, hydrocarbons)
#define TXPin        3   // GPS TX
#define RXPin        4   // GPS RX
#define MICS_PRE_PIN 8   // Heater power control
#define LED_PIN      9   // LED pin
#define PIN_SPI_CS   10  // SD card
#define ENS160_I2C_ADDRESS 0x53
#define BAUDRATE 9600

//Definitions MQ Sensors
#define Board "Arduino UNO"
#define Voltage_Resolution 5
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO

//Define Variables
float calcR0; //for MQ Sensors
float gasdata; //for MICS Sensor
float ENS160temperature = 25.0;
float ENS160humidity = 50.0;
float RatioMQ136CleanAir = 3.6; //RS / R0 = 3.6 ppm  
float RatioMQ4CleanAir = 4.4; //RS / R0 = 60 ppm 
unsigned long cycleInterval = 1000; // Changed from uint8_t
unsigned long warmupTime = 180000; // 180000 ms = 3 min, changed from uint8_t
float CO2_inertia = 0.99;
int CO2_tries = 100;
float MQ136_H2S_A = 36.737;
float MQ136_H2S_B = -3.536;
float MQ136_SO2_A = 503.34;
float MQ136_SO2_B = -3.774;
float MQ4_CH4_A = 1012.7;
float MQ4_CH4_B = -2.786;
bool ledState = 0;

//data
float lat = 0.0, lng = 0.0;
uint32_t m_date = 0, m_time = 0, tvoc = 0, eco2 = 0;
float co2 = 0.0, so2 = 0.0, h2s = 0.0, ch4 = 0.0, no2 = 0.0, c2h5oh = 0.0, h2 = 0.0, nh3 = 0.0, co = 0.0;
float pm25 = 0.0, pm10 = 0.0;

//Declare Sensor
File myFile;
SDS011 SDS011;
TinyGPSPlus GPS; // The TinyGPS++ object
SoftwareSerial GPS_SS(RXPin, TXPin); // The serial connection to the GPS device
CO2Sensor co2Sensor(MG811_PIN, CO2_inertia, CO2_tries);
DFRobot_ENS160_I2C ENS160(&Wire, ENS160_I2C_ADDRESS);
MICS_4514 MICS_4514(MICS_RED_PIN, MICS_NOX_PIN, MICS_PRE_PIN);
MQUnifiedsensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ4_PIN, "MQ-4");
MQUnifiedsensor MQ136(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ136_PIN, "MQ-136");

void setup() {
//Init serial port
    Serial.begin(BAUDRATE);
    GPS_SS.begin(BAUDRATE);

// Configure pin modes
    pinMode(MG811_PIN, INPUT);        // CO2 sensor
    pinMode(MQ136_PIN, INPUT);        // SO2 and H2S sensor
    pinMode(MQ4_PIN, INPUT);          // CH4 sensor
    pinMode(MICS_NOX_PIN, INPUT);     // NO2 sensor
    pinMode(MICS_RED_PIN, INPUT);     // CO and hydrocarbons sensor
    pinMode(MICS_PRE_PIN, OUTPUT);    // Heater power control
    pinMode(LED_PIN, OUTPUT);         // LED

// LED
    digitalWrite(LED_PIN, HIGH);

// SD card
    if (!SD.begin(PIN_SPI_CS)) {
        Serial.println(F("SD card initialization failed!"));
        while (true){
            Serial.println(F("SD card initialization failed!"));
            delay(1000);
        }
    }
    Serial.println(F("SD Card: initialization successful!"));

//read config
    String warmupTimeStr = readConfig("warmupTime");
    if (warmupTimeStr.length() > 0) {
        warmupTime = warmupTimeStr.toInt();
    }
    
    String ratioMQ136Str = readConfig("RatioMQ136CleanAir");
    if (ratioMQ136Str.length() > 0) {
        RatioMQ136CleanAir = ratioMQ136Str.toFloat();
    }
    
    String ratioMQ4Str = readConfig("RatioMQ4CleanAir");
    if (ratioMQ4Str.length() > 0) {
        RatioMQ4CleanAir = ratioMQ4Str.toFloat();
    }
    
    String tempStr = readConfig("ENS160temperature");
    if (tempStr.length() > 0) {
        ENS160temperature = tempStr.toFloat();
    }
    
    String humidityStr = readConfig("ENS160humidity");
    if (humidityStr.length() > 0) {
        ENS160humidity = humidityStr.toFloat();
    }

    String co2InertiaStr = readConfig("CO2_inertia");
    if (co2InertiaStr.length() > 0) {
        CO2_inertia = co2InertiaStr.toFloat();
    }

    String co2TriesStr = readConfig("CO2_tries");
    if (co2TriesStr.length() > 0) {
        CO2_tries = co2TriesStr.toInt();
    }

    String MQ136_H2S_A_Str = readConfig("MQ136_H2S_A");
    if (MQ136_H2S_A_Str.length() > 0) {
        MQ136_H2S_A = MQ136_H2S_A_Str.toFloat();
    }

    String MQ136_H2S_B_Str = readConfig("MQ136_H2S_B");
    if (MQ136_H2S_B_Str.length() > 0) {
        MQ136_H2S_B = MQ136_H2S_B_Str.toFloat();
    }

    String MQ136_SO2_A_Str = readConfig("MQ136_SO2_A");
    if (MQ136_SO2_A_Str.length() > 0) {
        MQ136_SO2_A = MQ136_SO2_A_Str.toFloat();
    }

    String MQ136_SO2_B_Str = readConfig("MQ136_SO2_B");
    if (MQ136_SO2_B_Str.length() > 0) {
        MQ136_SO2_B = MQ136_SO2_B_Str.toFloat();
    }

    String MQ4_CH4_A_Str = readConfig("MQ4_CH4_A");
    if (MQ4_CH4_A_Str.length() > 0) {
        MQ4_CH4_A = MQ4_CH4_A_Str.toFloat();
    }

    String MQ4_CH4_B_Str = readConfig("MQ4_CH4_B");
    if (MQ4_CH4_B_Str.length() > 0) {
        MQ4_CH4_B = MQ4_CH4_B_Str.toFloat();
    }

//SDS011 Setup
    SDS011.begin(0, 1); //RX, TX

//MG-811 Setup
    co2Sensor.calibrate();

//MICS-4514 Setup
    MICS_4514.setR0();

//ENS160 Setup
    ENS160.begin();
    ENS160.setPWRMode(ENS160_STANDARD_MODE);
    ENS160.setTempAndHum(ENS160temperature, ENS160humidity);

//MQ-136 Setup
    MQ136.init(); 
    MQ136.setRegressionMethod(1); //_PPM =  a*ratio^b
    Serial.print("MQ-136 Calibrating...");
    calcR0 = 0;
    for(int i = 1; i<=10; i ++){
        MQ136.update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += MQ136.calibrate(RatioMQ136CleanAir);}
    MQ136.setR0(calcR0/10);
    if(isinf(calcR0) || (calcR0 == 0)) {Serial.println("Invalid");} else Serial.println("  done!.");

//MQ-4 Setup
    MQ4.init(); 
    MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
    Serial.print("MQ-4 Calibrating...");
    calcR0 = 0;
    for(int i = 1; i<=10; i ++){
        MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
        calcR0 += MQ4.calibrate(RatioMQ4CleanAir);}
    MQ4.setR0(calcR0/10);
    if(isinf(calcR0) || (calcR0 == 0)) {Serial.println("Invalid");} else Serial.println("  done!.");

//Warmup
    Warmup(warmupTime);

//write data_title
    myFile = SD.open("data.txt", FILE_WRITE);
    if (myFile) {
        myFile.println("date , time , lat , lng , co2 , so2 , h2s , ch4 , no2 , c2h5oh , h2 , nh3 , co , tvoc , eco2 , pm25 , pm10");
        myFile.close();
    } else {
        Serial.println(F("SD Card: error on opening file data.txt"));
    }

//data_title
    Serial.println(F("date | time | lat | lng | co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co | tvoc | eco2 | pm25 | pm10"));
}

void loop() {
    // Read data from sensors
    readPM();
    readGPSData();
    co2 = readCO2();
    so2 = readSO2(MQ136_SO2_A, MQ136_SO2_B);
    h2s = readH2S(MQ136_H2S_A, MQ136_H2S_B);
    ch4 = readCH4(MQ4_CH4_A, MQ4_CH4_B);
    no2 = readNO2();
    c2h5oh = readC2H5OH();
    h2 = readH2();
    nh3 = readNH3();
    co = readCO();
    tvoc = readTVOC();
    eco2 = readeECO2();

    // Log data every cycleInterval - using proper time tracking
    static unsigned long lastCycleTime = 0;
    if (millis() - lastCycleTime >= cycleInterval) {
        lastCycleTime = millis();
        ledState = !ledState;
        writeData();
        logData();
        digitalWrite(LED_PIN, ledState);
    }
}

void Warmup(unsigned long warmupTime) {
    Serial.println("Starting sensor warmup sequence...");
    
    // Start MICS heater
    MICS_4514.setWarmupTime(warmupTime);
    MICS_4514.warmupStart();
    
    Serial.println("Waiting for sensors to preheat...");
    
    // Debugging MICS sensor warmup status
    Serial.print("MICS sensor ready state: ");
    Serial.println(MICS_4514.sensorReady() ? "READY" : "NOT READY");
    
    while(!MICS_4514.sensorReady()) {
        Serial.println("Preheating Gas Sensors...");
        delay(1000);
    }
    
    // Extra delay for MQ sensors to stabilize
    Serial.println("Almost done, final stabilization...");
    delay(5000);
    
    // Call setR0 one more time after we're fully stabilized
    Serial.println("Setting final R0 calibration values...");
    MICS_4514.setR0();
    
    Serial.println("Gas Sensors Preheated and Ready");
}

void readPM() {
    SDS011.read(&pm25, &pm10);
}

uint16_t readTVOC() {
    return (ENS160.getTVOC());
}

uint16_t readeECO2() {
    return (ENS160.getECO2());
}   

float readCO2() {
    return (co2Sensor.read());
}   

float readH2S(float A, float B) {
    MQ136.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ136.setA(A); MQ136.setB(B); // Configure the equation to calculate H2S
    return (MQ136.readSensor()); // H2S Concentration
}

float readSO2(float A, float B) {
    MQ136.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ136.setA(A); MQ136.setB(B); // Configure the equation to calculate SO2
    return (MQ136.readSensor()); // SO2 Concentration
}

float readCH4(float A, float B) {
    MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ4.setA(A); MQ4.setB(B); // Configure the equation to to calculate CH4
    return (MQ4.readSensor()); // CH4 Concentration
}

float readNO2() {
    gasdata = MICS_4514.getNitrogenDioxide();
    return (gasdata);
}

float readC2H5OH() {
    gasdata = MICS_4514.getEthanol();
    return (gasdata);
}

float readH2() {
    gasdata = MICS_4514.getHydrogen();
    return (gasdata);
}

float readNH3() {
    gasdata = MICS_4514.getAmmonia();
    return (gasdata);
}

float readCO() {
    gasdata = MICS_4514.getCarbonMonoxide();
    return (gasdata);
}

// Modified readGPSData() with robust validation
void readGPSData() {
  static unsigned long lastValidFix = 0;
  bool hasFix = false;

  // Process incoming GPS data
  while (GPS_SS.available() > 0) {
    if (GPS.encode(GPS_SS.read())) {
      if (GPS.location.isValid() && GPS.date.isValid() && GPS.time.isValid()) {
        hasFix = true;
        lastValidFix = millis();
      }
    }
  }

  // Only use data if we had a recent valid fix (within 2 seconds)
  if (hasFix || (millis() - lastValidFix < 2000)) {
    // Location data
    lat = GPS.location.lat();
    lng = GPS.location.lng();

    // Date handling (YYYYMMDD format)
    uint8_t day = GPS.date.day();
    uint8_t month = GPS.date.month();
    uint16_t year = GPS.date.year();

    // Enhanced date validation
    if (year >= 2020 && year <= 2100 &&
        month >= 1 && month <= 12 &&
        day >= 1 && day <= 31) {
      m_date = (year * 10000UL) + (month * 100UL) + day;
    }

    // Time handling with proper timezone math
    const float timezone = 5.5; // IST
    uint8_t hour = GPS.time.hour();
    uint8_t minute = GPS.time.minute();
    uint8_t second = GPS.time.second();

    // Convert all to seconds, add timezone offset
    uint32_t total_seconds = hour * 3600UL + minute * 60UL + second;
    total_seconds += (uint32_t)(timezone * 3600);

    // Handle day rollover
    while (total_seconds >= 86400) {
      total_seconds -= 86400;
      // Note: Would need to handle date increment here if needed
    }

    // Convert back to hours/minutes/seconds
    hour = total_seconds / 3600;
    total_seconds %= 3600;
    minute = total_seconds / 60;
    second = total_seconds % 60;

    // Final validation
    if (hour < 24 && minute < 60 && second < 60) {
      m_time = (hour * 10000UL) + (minute * 100UL) + second;
    }
  }
}

// Function to find a value by its name/key
String readConfig(const String &name) {
  String value = ""; // Default empty value
  myFile = SD.open("config.txt", FILE_READ);
  if (!myFile) {
    Serial.println(F("SD Card: error on opening file config.txt"));
    return value; // Return empty value if file can't be opened
  }
  
  while (myFile.available()) {
    String line = myFile.readStringUntil('\n');
    line.trim(); // Remove any leading/trailing whitespace
    // Find the separator position
    int separatorPos = line.indexOf('=');
    if (separatorPos > 0) {
      String key = line.substring(0, separatorPos);
      key.trim();
      
      if (key == name) {
        value = line.substring(separatorPos + 1);
        value.trim();
        break; // Found the value, exit the loop
      }
    }
  }
  
  myFile.close(); // Always close the file
  return value;
}

void logData() {
    Serial.print(m_date); Serial.print(" | ");
    Serial.print(m_time); Serial.print(" | ");
    Serial.print(lat, 4); Serial.print(" | ");
    Serial.print(lng, 4); Serial.print(" | ");
    Serial.print(co2); Serial.print(" | ");
    Serial.print(so2); Serial.print(" | ");
    Serial.print(h2s); Serial.print(" | ");
    Serial.print(ch4); Serial.print(" | ");
    Serial.print(no2); Serial.print(" | ");
    Serial.print(c2h5oh); Serial.print(" | ");
    Serial.print(h2); Serial.print(" | ");
    Serial.print(nh3); Serial.print(" | ");
    Serial.print(co); Serial.print(" | ");
    Serial.print(tvoc); Serial.print(" | ");
    Serial.print(eco2); Serial.print(" | ");
    Serial.print(pm25); Serial.print(" | ");
    Serial.print(pm10); Serial.println();
}

void writeData() {
    myFile = SD.open("data.txt", FILE_WRITE);
    if (myFile) {
        myFile.print(m_date); myFile.print(",");
        myFile.print(m_time); myFile.print(",");
        myFile.print(lat, 6); myFile.print(",");
        myFile.print(lng, 6); myFile.print(",");
        myFile.print(co2); myFile.print(",");
        myFile.print(so2); myFile.print(",");
        myFile.print(h2s); myFile.print(",");
        myFile.print(ch4); myFile.print(",");
        myFile.print(no2); myFile.print(",");
        myFile.print(c2h5oh); myFile.print(",");
        myFile.print(h2); myFile.print(",");
        myFile.print(nh3); myFile.print(",");
        myFile.print(co); myFile.print(",");
        myFile.print(tvoc); myFile.print(",");
        myFile.print(eco2); myFile.print(",");
        myFile.print(pm25); myFile.print(",");
        myFile.print(pm10); myFile.println();
        myFile.close();
    } else {
        Serial.println(F("SD Card: error on opening file data.txt"));
    }
}
