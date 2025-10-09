/*
  MQ Sensor: R0 Calibration + PPM Calculation (Jaycon Method, No EEPROM)
  Board: Arduino Uno / Nano / ESP32 (adjust ADC bits)
  Sensor: MQ-series (e.g., MQ-135 for CO2)
  
  Wiring:
    - Sensor analog output → A0
    - RL (load resistor) between Vout and GND (typically 10k)
    - VC = 5.0V (use 3.3V for ESP32)
*/
#include <driver/adc.h>
#include <esp_adc_cal.h>

esp_adc_cal_characteristics_t adc_chars;

const int MQ_PIN = 34;       // Analog input pin
const float VC = 3.3;        // Supply voltage (V)
const float RL = 1000.0;    // Load resistor (Ω)
const int ADC_MAX = 4095;    // 10-bit ADC (use 4095 for ESP32 12-bit ADC)

// CO2 parameters (from MQ135 datasheet)
const float CLEAN_AIR_RATIO = 3.6;  // Rs/R0 ratio in clean air for MQ135
const float M = -0.42;             // slope
const float B = 1.3;              // intercept

// Calibration parameters
const int CALIB_READS = 50;         // Number of samples for R0 calibration
const unsigned long CALIB_DELAY_MS = 500;

float R0 = -1.0;

// --- Convert ADC reading to voltage ---
float analogToVoltage(int raw) {
  const float Correction_factor = 1100.0000 / adc_chars.vref;
  return (map(raw, 4095, 0, 0, 4095) / (float)ADC_MAX) * VC * Correction_factor;
}

// --- Calculate sensor resistance Rs ---
float calculateRs(int raw) {
  if (raw <= 0) return 1e9;
  float vRL = analogToVoltage(raw);
  float rs = (VC * RL / vRL) - RL;
  return rs;
}

// --- Calculate CO2 ppm from Rs and R0 ---
float calculatePPM(float rs, float r0) {
  if (rs <= 0 || r0 <= 0) return -1;
  float ratio = rs / r0;
  float log_ratio = log10(ratio);
  float log_ppm = (log_ratio - B) / M;
  float ppm = pow(10.0, log_ppm);
  return ppm;
}

// --- Calibrate R0 in clean air ---
float calibrateR0() {
  Serial.println(F("Calibrating... place sensor in clean air"));
  float rs_sum = 0;
  for (int i = 0; i < CALIB_READS; i++) {

  long Sum = 0;
  const int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(ADC1_CHANNEL_6);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;

    float rs = calculateRs(AVG_Raw_AnalogValue);

    rs_sum += rs;
    delay(CALIB_DELAY_MS);
  }
  float rs_avg = rs_sum / CALIB_READS;
  float r0_calc = rs_avg / CLEAN_AIR_RATIO;
  Serial.print(F("Calibration done. R0 = "));
  Serial.print(r0_calc);
  Serial.println(F(" ohms"));
  return r0_calc;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n--- MQ135 CO2 Sensor ---"));
    esp_adc_cal_characterize(ADC_UNIT_1,
                           ADC_ATTEN_DB_12,
                           ADC_WIDTH_BIT_12,
                           0, &adc_chars);
  // Perform calibration once at startup
  R0 = calibrateR0();
}

void loop() {
  long Sum = 0;
  const int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(ADC1_CHANNEL_6);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;

  float rs = calculateRs(AVG_Raw_AnalogValue);
  float ratio = rs / R0;
  float ppm = calculatePPM(rs, R0);

  Serial.print(F("R0="));
  Serial.print(R0);
  Serial.print(F("  VRL="));
  Serial.print(analogToVoltage(AVG_Raw_AnalogValue), 3);
  Serial.print(F("V  RS="));
  Serial.print(rs, 1);
  Serial.print(F("Ω  Ratio="));
  Serial.print(ratio, 3);
  Serial.print(F("  CO2="));

  if (ppm > 0 && ppm < 1e6) Serial.print(ppm, 2);
  else Serial.print(F("N/A"));
  Serial.println(F(" ppm"));

  delay(1000);
}
