#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 35
#define ONE_WIRE_BUS 22

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Dallas Temperature Sensor Test");
  sensors.begin();  // Initialize sensor
}

void loop() {
  sensors.requestTemperatures();  // Send command to get temperatures
  float tempC = sensors.getTempCByIndex(0);  // Read temperature from the first sensor

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Sensor not found or disconnected!");
  } else {
    float tempF = sensors.toFahrenheit(tempC);
    Serial.print("Temperature = ");
    Serial.print(tempC);
    Serial.print(" °C, ");
    Serial.print(tempF);
    Serial.println(" °F");
  }

  delay(1000);  // Wait before next reading
}
