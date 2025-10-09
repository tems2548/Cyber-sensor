//--------------include lib from datasheet--------------------//
#include <driver/adc.h>
#include <esp_adc_cal.h>
//------------------------------------------------------------//

esp_adc_cal_characteristics_t adc_chars;

float Unadjusted_ADC_Read_I(adc1_channel_t Raw_AnalogPIN){
  long Sum = 0.00;
  int Number_of_Sample = 100;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
 
  // Calculate voltageADC
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;
  //1.5300
  const float offset = 1.5300; // find default offset voltage at no current
  float Real_voltage = VoltageADC - offset; 
  float Curent = Real_voltage / 0.185; // 185mV per A for 5A version
  return Curent;
}
void setup() {

  Serial.begin(115200);
  esp_adc_cal_characterize(ADC_UNIT_1,
                           ADC_ATTEN_DB_12,
                           ADC_WIDTH_BIT_12,
                           0, &adc_chars);


}

void loop() {
 float raw = Unadjusted_ADC_Read_I(ADC1_CHANNEL_7);
 delay(400);
 Serial.println("Current =  = " + String(raw, 4));
}
