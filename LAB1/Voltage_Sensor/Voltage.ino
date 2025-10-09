#include <driver/adc.h>
#include <esp_adc_cal.h>
bool DEBUG = true;

const float R1 = 29860.00000; // Resistor 1
const float R2 = 7450.00000;  // Resistor 2

esp_adc_cal_characteristics_t adc_chars;
float Unadjusted_ADC_Read(adc1_channel_t Raw_AnalogPIN)
{  delay(100);

  const float VoltageADC = ((4096.000 - adc1_get_raw(Raw_AnalogPIN)) * 3.3) / 4096.00;
  const float Voltage = VoltageADC * ((R1 + R2) / R2);
  return Voltage;
}
void setup()
{
  Serial.begin(9600);

   
  // Setup analog pin
  esp_adc_cal_characterize(ADC_UNIT_1,
                           ADC_ATTEN_DB_12,
                           ADC_WIDTH_BIT_12,
                           0, &adc_chars);
}
void loop()
{ 
  float raw = Unadjusted_ADC_Read(ADC1_CHANNEL_6);
  if (DEBUG == true)
  {
    Serial.println("");
    delay(1000);
    Serial.println("Uncalibration_voltage = " + String(raw, 4));
    Serial.print("-------------------------------------------------------------");
    Serial.println("");
  }
}
