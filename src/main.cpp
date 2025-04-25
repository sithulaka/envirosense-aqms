#include <Wire.h>
#include <SoftwareSerial.h>

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
#define MICS_PRE_PIN 8  // Heater power control
#define PIN_SPI_CS   10 // SD card
#define ENS160_I2C_ADDRESS 0x53
#define cycleInterval 1000

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
uint8_t warmupTime = 180000; // 180000 ms = 3 min

//data
float lat, lng;
uint16_t m_date, m_time, tvoc, eco2;
float co2, so2, h2s, ch4, no2, c2h5oh, h2, nh3, co;
float pm25, pm10;

//Declare Sensor
File myFile;
SDS011 SDS011;
TinyGPSPlus GPS; // The TinyGPS++ object
SoftwareSerial GPS_SS(RXPin, TXPin); // The serial connection to the GPS device
CO2Sensor co2Sensor(MG811_PIN, 0.99, 100);
DFRobot_ENS160_I2C ENS160(&Wire, ENS160_I2C_ADDRESS);
MICS_4514 MICS_4514(MICS_RED_PIN, MICS_NOX_PIN, MICS_PRE_PIN);
MQUnifiedsensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ4_PIN, "MQ-4");
MQUnifiedsensor MQ136(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ136_PIN, "MQ-136");

void setup() {
//Init serial port
    Serial.begin(9600);
    GPS_SS.begin(9600);

// SD card
    if (!SD.begin(PIN_SPI_CS)) {
        Serial.println(F("SD card initialization failed!"));
        while (true){
            Serial.println(F("SD card initialization failed!"));
            delay(1000);
        }
    }

//read config
    uint8_t warmupTime = readConfig("warmupTime").toInt();
    float RatioMQ136CleanAir = readConfig("RatioMQ136CleanAir").toFloat();
    float RatioMQ4CleanAir = readConfig("RatioMQ4CleanAir").toFloat();
    float ENS160temperature = readConfig("ENS160temperature").toFloat();
    float ENS160humidity = readConfig("ENS160humidity").toFloat();

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
        while (true){
            Serial.println(F("SD Card: error on opening file data.txt"));
            delay(1000);
        }
    }

//data_title
    Serial.println(F("date | time | lat | lng | co2 | so2 | h2s | ch4 | no2 | c2h5oh | h2 | nh3 | co | tvoc | eco2 | pm25 | pm10"));
}

void loop() {

// Read data from sensors
    readPM();
    readGPSlat();
    readGPSlng();
    readGPSdate();
    readGPStime();
    co2 = readCO2();
    so2 = readSO2();
    h2s = readH2S();
    ch4 = readCH4();
    no2 = readNO2();
    c2h5oh = readC2H5OH();
    h2 = readH2();
    nh3 = readNH3();
    co = readCO();
    tvoc = readTVOC();
    eco2 = readeECO2();

// Log data every cycleInterval
    static unsigned long lastCycleTime = 0;
    if (millis() - lastCycleTime >= cycleInterval) {
        lastCycleTime = millis();
        writeData();
        logData();
    }
}

void Warmup(uint8_t warmupTime) {
    MICS_4514.setWarmupTime(warmupTime);
    MICS_4514.warmupStart();
    while(!MICS_4514.sensorReady()) {
        Serial.println("Preheating Gas Sensors...");
    }
    delay(warmupTime);
    Serial.println("Gas Sensors Preheated");
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

float readH2S() {
    MQ136.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ136.setA(36.737); MQ136.setB(-3.536); // Configure the equation to calculate H2S
    return (MQ136.readSensor()); // H2S Concentration
}

float readSO2() {
    MQ136.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ136.setA(503.34); MQ136.setB(-3.774); // Configure the equation to calculate SO2
    return (MQ136.readSensor()); // SO2 Concentration
}

float readCH4() {
    MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
    MQ4.setA(1012.7); MQ4.setB(-2.786); // Configure the equation to to calculate CH4
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

float readCH42() {
    gasdata = MICS_4514.getMethane();
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

// Reads and updates all GPS data
void readGPSData() {
  while (GPS_SS.available() > 0) {
    GPS.encode(GPS_SS.read());  // Feed GPS data to TinyGPS++
  }
  if (GPS.location.isUpdated()) {
    lat = GPS.location.lat();
    lng = GPS.location.lng();
  }
  if (GPS.date.isUpdated()) {
    m_date = GPS.date.value();
  }
  if (GPS.time.isUpdated()) {
    m_time = GPS.time.value();
  }
}

// Function to find a value by its name/key
String readConfig(const String &name) {
  myFile = SD.open("config.txt", FILE_READ);
  if (myFile) {
    while (myFile.available()) {
      String line = myFile.readStringUntil('\n');
      line.trim(); // Remove any leading/trailing whitespace
      // Find the separator position
      int separatorPos = line.indexOf('=');
      if (separatorPos > 0) {
        String key = line.substring(0, separatorPos);
        key.trim();
        
        if (key == name) {
          String value = line.substring(separatorPos + 1);
          value.trim();
          myFile.close();
          return value;
        }
      }
    }
    myFile.close();
  }
}

void logData() {
    Serial.print(date); Serial.print(" | ");
    Serial.print(time); Serial.print(" | ");
    Serial.print(lat); Serial.print(" | ");
    Serial.print(lng); Serial.print(" | ");
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
        myFile.print(date); myFile.println(",");
        myFile.print(time); myFile.println(",");
        myFile.print(lat); myFile.println(",");
        myFile.print(lng); myFile.println(",");
        myFile.print(co2); myFile.println(",");
        myFile.print(so2); myFile.println(",");
        myFile.print(h2s); myFile.println(",");
        myFile.print(ch4); myFile.println(",");
        myFile.print(no2); myFile.println(",");
        myFile.print(c2h5oh); myFile.println(",");
        myFile.print(h2); myFile.println(",");
        myFile.print(nh3); myFile.println(",");
        myFile.print(co); myFile.println(",");
        myFile.print(tvoc); myFile.println(",");
        myFile.print(eco2); myFile.println(",");
        myFile.print(pm25); myFile.println(",");
        myFile.print(pm10); myFile.println(",");
        myFile.close();
    } else {
        Serial.println(F("SD Card: error on opening file data.txt"));
    }
}
