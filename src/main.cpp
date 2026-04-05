#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <LiquidCrystal_I2C.h>

// --- Hardware Object Instantiation ---
Adafruit_ADS1115 ads; // Initialize the 16-bit ADC at default I2C address 0x48
LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD at I2C address 0x27 (16 cols, 2 rows)

// --- System Constants & Calibration ---
const float REFERENCE_RESISTOR = 1000.0; // Your 1k Ohm reference resistor in the voltage divider
const float VDD = 5.0; // The theoretical 5V supply from the Arduino
const float RTD_NOMINAL = 100.0; // Resistance of PT100 at 0 degrees Celsius

// --- Non-Blocking Timer Variables ---
unsigned long previousMillis = 0;
const long pollingInterval = 500; // Poll data and update screen every 500 milliseconds

void setup() {
  Serial.begin(9600);
  
  // Initialize I2C Bus for both devices
  Wire.begin();
  
  // Boot LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System Booting..");
  
  // Boot ADS1115
  // We use 1x gain (+/- 4.096V) since our voltage divider will output around 0.45V
  ads.setGain(GAIN_ONE); 
  if (!ads.begin()) {
    lcd.clear();
    lcd.print("ADC Error!");
    while (1); // Halt execution on hardware failure
  }
  
  delay(1000); // Short blocking delay only acceptable during boot sequence
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();

  // Execute this block strictly every 500ms without halting the microcontroller
  if (currentMillis - previousMillis >= pollingInterval) {
    previousMillis = currentMillis;

    // 1. Data Acquisition: Read raw ADC value from Pin A0 on the ADS1115
    int16_t adc0 = ads.readADC_SingleEnded(0);
    
    // 2. Voltage Conversion: Convert raw ADC integer to measurable voltage
    float volts0 = ads.computeVolts(adc0);

    // 3. Resistance Calculation: Solve the Voltage Divider equation for R2 (the RTD)
    // Formula: R_rtd = R_ref * (V_measured / (VDD - V_measured))
    float rtdResistance = REFERENCE_RESISTOR * (volts0 / (VDD - volts0));

    // 4. Temperature Derivation: Linear approximation for PT100 (MVP Level)
    // A PT100 increases by approx 0.385 ohms per degree Celsius.
    float temperatureC = (rtdResistance - RTD_NOMINAL) / 0.385;

    // 5. Output to Display
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatureC, 2); // Display to 2 decimal places
    lcd.print(" C   "); // Trailing spaces to overwrite old, longer strings

    lcd.setCursor(0, 1);
    lcd.print("Res: ");
    lcd.print(rtdResistance, 1);
    lcd.print(" Ohm ");

    // Optional: Output to Serial Monitor for debugging / logging
    Serial.print("V: "); Serial.print(volts0, 4);
    Serial.print(" | R: "); Serial.print(rtdResistance, 2);
    Serial.print(" | T: "); Serial.println(temperatureC, 2);
  }
}