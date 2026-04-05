#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Hardware Mapping ---
#define ONE_WIRE_BUS 2 // The digital pin connected to the DS18B20 Yellow Data Wire

// --- Object Instantiation ---
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Standard I2C address 0x27 for a 16x2 display

// --- Deterministic Execution Variables ---
unsigned long previousMillis = 0;
const long pollingInterval = 1000; // Update the screen and fetch data every 1 second
float currentTempC = 0.0; // Global variable to hold the latest valid data

void setup() {
  Serial.begin(9600);
  
  // 1. Initialize Display
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting Core...");

  // 2. Initialize Sensor Bus
  sensors.begin();
  
  // 3. The Low-Latency Configuration (Crucial for MVP Performance)
  // This tells the sensor to calculate temperature in the background without pausing the Arduino
  sensors.setWaitForConversion(false); 
  
  // Set to 12-bit resolution for maximum precision (0.0625°C increments)
  sensors.setResolution(12); 

  // Send the very first background request before the loop starts
  sensors.requestTemperatures(); 
  
  delay(1000); // Allow hardware to stabilize
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // Execute this block strictly every 1000ms without blocking the processor
  if (currentMillis - previousMillis >= pollingInterval) {
    previousMillis = currentMillis;

    // 1. Data Retrieval: Pull the temperature calculated during the LAST 1000ms
    // Index 0 refers to the first sensor on the wire.
    currentTempC = sensors.getTempCByIndex(0);

    // 2. Data Request: Tell the sensor to start calculating the NEXT temperature in the background
    sensors.requestTemperatures();

    // 3. Error Handling: The sensor returns -127.00 if the wire is disconnected
    if (currentTempC <= -100.0) {
      lcd.setCursor(0, 0);
      lcd.print("SENSOR FAULT!   ");
      lcd.setCursor(0, 1);
      lcd.print("Check Wiring.   ");
      Serial.println("ERR: DS18B20 Open Circuit Detected.");
    } else {
      // 4. Data Output to Screen
      lcd.setCursor(0, 0);
      lcd.print("Water Temp:     "); // Trailing spaces clear old characters
      
      lcd.setCursor(0, 1);
      lcd.print(currentTempC, 2); // Render to 2 decimal places
      lcd.print(" ");
      lcd.print((char)223); // Renders the physical degree symbol '°'
      lcd.print("C       ");
      
      // Output to Serial for logging
      Serial.print("Data Stream -> Temp: ");
      Serial.print(currentTempC, 4); // Serial outputs deeper precision
      Serial.println(" C");
    }
  }
  
  // Because we used setWaitForConversion(false) and millis(), 
  // you can safely add other logic here (like reading a flow sensor or pressing buttons)
  // and the processor will respond instantly.
}