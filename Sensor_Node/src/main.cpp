#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Adafruit_INA219.h>
#include <Wire.h> //for I^2C communication
#include "ThingSpeak.h"
#include <WiFi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

//wifi password
char ssid[] = "123";   // input ssid
char pass[] = "123";   // input network password

//thingspeak settup
WiFiClient  client;
unsigned long myChannelNumber = 123; //input channel number
const char * myWriteAPIKey = "123"; //input write api key

//sensors
Adafruit_INA219 ina219;
Adafruit_BME280 bme280;  
const int voltage_pin = 34;

//LCD screen
#define TFT_CS 5
#define TFT_A0 16
#define TFT_RESET 17
Adafruit_ST7735 tft(TFT_CS, TFT_A0, TFT_RESET);

//ina219 variables
float voltage_V = 0;
float current_mA = 0;
float power_mW = 0;

//bme280 variables
float temp = 0;
float humidity = 0;
float pressure = 0;

//voltage divider
const float R1 = 270000.0;
const float R2 = 10000.0;
const float max_adc = 4095.0; //3.3 in bits
const float vref = 3.3; //max voltage of esp32

void setup(){
  
  Serial.begin(9600); //baud for esp32
  delay(1000);

  //LCD screen setup
  tft.initR(INITR_REDTAB);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 0);
  tft.println("Booting...");

  //wifi setup
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  tft.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tft.print(".");
  }
  Serial.println("\nWiFi Connected");
  tft.println("WiFi Connected");
  ThingSpeak.begin(client);


  //sensor setup
  Wire.begin(); //will default to gpio 21 =sda and gpio 22 =scl since those are for sda and scl on esp32

  //check if ina219 is connected
  if(!ina219.begin()){
    Serial.println("Failed to find INA219");
    tft.println("Failed to find INA219");
  }

  Serial.println("INA219 detected");
  tft.println("INA219 detected");

  //check if bme280 is connected

  if(!bme280.begin(0x76)){
    Serial.println("Failed to find BME280");
    tft.println("Failed to find BME280");
  }
  Serial.println("BME280 detected");
  tft.println("BME280 detected");
}

float readvoltage(){
  int adc = analogRead(voltage_pin);
  float vout = (adc/max_adc) * vref; //calculate output voltage by taking the the percentage of the max voltage
  float vin  = vout * ((R1+R2)/R2);
  return vin;
}


void loop(){

  voltage_V = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  temp = bme280.readTemperature();
  humidity = bme280.readHumidity();
  pressure = bme280.readPressure();

  float divider_voltage = readvoltage();

  Serial.println("========================");
  Serial.printf("Divider Voltage: %.2f V\n", divider_voltage);
  Serial.printf("Voltage: %.2f V\n", voltage_V);
  Serial.printf("Current: %.2f mA\n", current_mA);
  Serial.printf("Power: %.2f mW\n", power_mW);
  Serial.printf("Temperature: %.2f °C\n", temp);
  Serial.printf("Humidity: %.2f %\n", humidity);
  Serial.printf("Pressure: %.2f hPa\n", pressure);
  Serial.println("========================");


  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("Divider Voltage:");
  tft.print(divider_voltage);
  tft.print("V\n\n");

  tft.print("Voltage:");
  tft.print(voltage_V);
  tft.print("V\n\n");

  tft.print("Current:");
  tft.print(current_mA);
  tft.print("mA\n\n");

  tft.print("Power:");
  tft.print(power_mW);
  tft.print("mW\n\n");

  tft.print("Temperature:");
  tft.print(temp);
  tft.print("C\n\n");

  tft.printf("Humidity: ");
  tft.print(humidity);
  tft.print("%\n\n");

  tft.print("Pressure:");
  tft.print(pressure);
  tft.print("hPa\n\n");
 

  ThingSpeak.setField(1, divider_voltage);
  ThingSpeak.setField(2, voltage_V);
  ThingSpeak.setField(3, current_mA);
  ThingSpeak.setField(4, power_mW);
  ThingSpeak.setField(5, temp);
  ThingSpeak.setField(6, humidity);
  ThingSpeak.setField(7, pressure);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("ThingSpeak update successful");

  } else {
    Serial.println("ThingSpeak update failed. Code: " + String(x));

  }

  delay(20000);


  
}