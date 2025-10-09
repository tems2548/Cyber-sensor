#include <Arduino.h>

//--------------include lib from datasheet--------------------//
#include <driver/adc.h>
#include <esp_adc_cal.h>
//------------------------------------------------------------//

// define ADC pin
#define ADC_pin 34
#define ADC2_pin 35

bool DEBUG = true;

const float R1 = 29860.00000; // Resistor 1
const float R2 = 7450.00000;  // Resistor 2

esp_adc_cal_characteristics_t adc_chars;

float determinant3x3(float cof[3][3]){

  // For a matrix A = 
  // | a b c |
  // | d e f |
  // | g h i |
  // The determinant |A| = a(ei − fh) − b(di − fg) + c(dh − eg)

  return cof[0][0]*(cof[1][1]*cof[2][2]-cof[1][2]*cof[2][1])
       - cof[0][1]*(cof[1][0]*cof[2][2]-cof[1][2]*cof[2][0])
       + cof[0][2]*(cof[1][0]*cof[2][1]-cof[1][1]*cof[2][0]);
}

float Unadjusted_ADC_Read_V(adc1_channel_t Raw_AnalogPIN)
{  delay(100);
  long Sum = 0;
  const int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;
  const float divider_gain = (R1 + R2) / R2;
  const float Voltage = VoltageADC * divider_gain;
  return Voltage;
}

float SingleP_Calibration_voltage_Read(
    adc1_channel_t Raw_AnalogPIN,
    float Vtrue,
    float Vmeter,
    float Manual_calibration)
{
  // find Average value
  long Sum = 0;
  const int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;

  // Calculate voltage
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;

  // Calculate Divider gain
  const float divider_gain = (R1 + R2) / R2;

  // Calculate Calibration factor
  const float Calibration_factor = Vtrue / Vmeter;

  // Calculate Correction_factor
  const float Correction_factor = 1100.0000 / adc_chars.vref;

  // Calculate manual Calibration
  const float Manual_calibrate = Manual_calibration; // Adjust for ultimate accuracy if reading too high then use e.g. 0.99, too low use 1.01

  // Calculate Calibration_Voltage
  const float Calibration_Voltage = VoltageADC * divider_gain * Calibration_factor * Manual_calibrate * Correction_factor;
  return Calibration_Voltage;
}
float Linear_Calibration_voltage_Read(
    adc1_channel_t Raw_AnalogPIN,
    float Vtrue_1,
    float Vtrue_2,
    float Vmeter_1,
    float Vmeter_2,
    float Manual_calibration)
{
  // find Average value
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
 
  // Calculate voltage
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;
  //uint32_t Vadc_pin_mv = esp_adc_cal_raw_to_voltage((4095.000 - AVG_Raw_AnalogValue), &adc_chars);
  //float VoltageADC = Vadc_pin_mv / 1000.0f;

  // Calculate Divider gain
  const float divider_gain = (R1 + R2) / R2;

  // Calculate slope and find offset coefficient
  float slope;
  float offset;

  if (fabs(Vmeter_2 - Vmeter_1) < 1e-6)
  {
    slope = 1.0f;
    offset = 0.0f;
  }
  else
  {
    slope = (Vtrue_2 - Vtrue_1) / (Vmeter_2 - Vmeter_1);
    offset = Vtrue_1 - (slope * Vmeter_1);
  }

  // Calculate Correction_factor
  const float Correction_factor = 1100.0000 / adc_chars.vref;

  // Calculate Calibration_Voltage

  const float Calibration_Voltage = (slope * (VoltageADC * divider_gain)* Manual_calibration) + offset;
  return Calibration_Voltage;
}

float Non_Linear_Calibration_voltage_Read(
    adc1_channel_t Raw_AnalogPIN,
    float Vtrue_1,
    float Vtrue_2,
    float Vtrue_3,
    float Vmeter_1,
    float Vmeter_2,
    float Vmeter_3,
    float Manual_calibration)
{
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
 
  // Calculate voltage
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;
  //uint32_t Vadc_pin_mv = esp_adc_cal_raw_to_voltage((4095.000 - AVG_Raw_AnalogValue), &adc_chars);
  //float VoltageADC = Vadc_pin_mv / 1000.0f;

  // Calculate Divider gain
  const float divider_gain = (R1 + R2) / R2;
  
  const float Vmeter_1_power = Vmeter_1*Vmeter_1;
  const float Vmeter_2_power = Vmeter_2*Vmeter_2;
  const float Vmeter_3_power = Vmeter_3*Vmeter_3;
  // (x,y) = (Vmeter,Vtrue)
  // |     Vtrue1 = a(Vmeter_1)^2 + (β * Vmeter_1) + γ ---1
  // |     Vtrue2 = a(Vmeter_2)^2 + (β * Vmeter_2) + γ ---2
  // |     Vtrue3 = a(Vmeter_3)^2 + (β * Vmeter_3) + γ ---3
  //get value from system of equation

  float main_mat[3][3] = {{Vmeter_1_power, Vmeter_1, 1.000},
                          {Vmeter_2_power, Vmeter_2, 1.000},
                          {Vmeter_3_power, Vmeter_3, 1.000}};

  float MAT_A[3][3]    = {{Vtrue_1, Vmeter_1, 1.000},
                          {Vtrue_2, Vmeter_2, 1.000},
                          {Vtrue_3, Vmeter_3, 1.000}};

  float MAT_B[3][3]    = {{Vmeter_1_power, Vtrue_1, 1.000},
                          {Vmeter_2_power, Vtrue_2, 1.000},
                          {Vmeter_3_power, Vtrue_3, 1.000}};

  float MAT_C[3][3]    = {{Vmeter_1_power, Vmeter_1, Vtrue_1},
                          {Vmeter_2_power, Vmeter_2, Vtrue_2},
                          {Vmeter_3_power, Vmeter_3, Vtrue_3}};
                        
  //solve for coefficients "α" , "β" , "γ" by Cramer's rule
  
  float denominator = determinant3x3(main_mat);

  const float A = determinant3x3(MAT_A) / denominator;
  const float B = determinant3x3(MAT_B) / denominator;
  const float C = determinant3x3(MAT_C) / denominator;

  // Calculate Correction_factor
  const float Correction_factor = 1100.0000 / adc_chars.vref;

  //calculate Calibration Voltage
  const float V_Unadjust = VoltageADC * divider_gain * Correction_factor;
  const float Calibration_Voltage = ((A*V_Unadjust*V_Unadjust) + (B*V_Unadjust) + C ) * Manual_calibration;
  return Calibration_Voltage;
}
float ESP32_lib_Calibration(adc1_channel_t Raw_AnalogPIN){
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
  
  // Calculate voltage
  const uint32_t Vadc_pin_mv = esp_adc_cal_raw_to_voltage(map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095), &adc_chars);
  const float Calibration_Voltage = Vadc_pin_mv / 1000.0f;
  return Calibration_Voltage;
}

float Map_value_Calibration(adc1_channel_t Raw_AnalogPIN,
                            long ADC_raw_min,
                            long ADC_raw_max)
{
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;

  //map() = first linear approximation.
  //offset correction = fine-tuning to reduce error.

  const float mV = map(AVG_Raw_AnalogValue, ADC_raw_min, ADC_raw_max, 0, 16500);
  const float offset = map(mV, 16500, 0, -1000, 1000);

  const float Calibration_Voltage = (mV + offset) / 1000.0f;
  return Calibration_Voltage;
}
float Unadjusted_ADC_Read_I(adc1_channel_t Raw_AnalogPIN){
  //const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.3) / 4096.00;
  //(voutI - vRef) / sensivity
  //const float Voltage = VoltageADC * ((R1 + R2) / R2);
  
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
  // if (Real_voltage < 0.00 || Curent < 0.00){
  //   Real_voltage = 0.00;
  //   Curent = 0.00;
  // }
  return Curent;
}
float Linear_Calibration_Current_Read(
    adc1_channel_t Raw_AnalogPIN,
    float Ctrue_1,
    float Ctrue_2,
    float Cmeter_1,
    float Cmeter_2,
    float Manual_calibration)
{
  // find Average value
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
 
  // Calculate voltage
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;

  // Calculate slope and find offset coefficient
  float slope;
  float offset;
  //1.5300
  const float offset_C = 1.5300; // find default offset voltage at no current
  float Real_voltage = VoltageADC - offset_C; 
  float Current = Real_voltage / 0.185; // 185mV per A for 5A version
  // if (Real_voltage < 0.00 || Curent < 0.00){
  //   Real_voltage = 0.00;
  //   Curent = 0.00;
  // }

  if (fabs(Cmeter_2 - Cmeter_1) < 1e-6)
  {
    slope = 1.0f;
    offset = 0.0f;
  }
  else
  {
    slope = (Ctrue_2 - Ctrue_1) / (Cmeter_2 - Cmeter_1);
    offset = Ctrue_1 - (slope * Cmeter_1);
  }

  // Calculate Correction_factor
  const float Correction_factor = 1100.0000 / adc_chars.vref;

  // Calculate Calibration_Current

  const float Calibration_Current = (slope * Current * Manual_calibration) + offset;
  return Calibration_Current;
}
float data = 0.00;
void setup()
{
  Serial.begin(9600);
  //adc1_config_width(ADC_WIDTH_BIT_12); // set 12 bit ADC resolution
  // Setup analog pin
  esp_adc_cal_characterize(ADC_UNIT_1,
                           ADC_ATTEN_DB_12,
                           ADC_WIDTH_BIT_12,
                           0, &adc_chars);
}

float Non_Linear_Calibration_Current_Read(
    adc1_channel_t Raw_AnalogPIN,
    float Ctrue_1,
    float Ctrue_2,
    float Ctrue_3,
    float Cmeter_1,
    float Cmeter_2,
    float Cmeter_3,
    float Manual_calibration)
{
  long Sum = 0;
  int Number_of_Sample = 100.000;
  for (int sample = 0; sample < Number_of_Sample; sample++)
  {
    Sum += adc1_get_raw(Raw_AnalogPIN);
  }
  const float AVG_Raw_AnalogValue = Sum / (float)Number_of_Sample;
 
  // Calculate voltage
  const float VoltageADC = (map(AVG_Raw_AnalogValue, 4095, 0, 0, 4095) * 3.300) / 4096.000;

  // Calculate Divider gain
  //const float divider_gain = (R1 + R2) / R2;
  const float offset_C = 1.5300; // find default offset voltage at no current
  float Real_voltage = VoltageADC - offset_C; 
  float Current = Real_voltage / 0.185; 

  const float Cmeter_1_power = Cmeter_1*Cmeter_1;
  const float Cmeter_2_power = Cmeter_2*Cmeter_2;
  const float Cmeter_3_power = Cmeter_3*Cmeter_3;
  // (x,y) = (Vmeter,Vtrue)
  // |     Vtrue1 = a(Vmeter_1)^2 + (β * Vmeter_1) + γ ---1
  // |     Vtrue2 = a(Vmeter_2)^2 + (β * Vmeter_2) + γ ---2
  // |     Vtrue3 = a(Vmeter_3)^2 + (β * Vmeter_3) + γ ---3
  //get value from system of equation

  float main_mat[3][3] = {{Cmeter_1_power, Cmeter_1, 1.000},
                          {Cmeter_2_power, Cmeter_2, 1.000},
                          {Cmeter_3_power, Cmeter_3, 1.000}};

  float MAT_A[3][3]    = {{Ctrue_1, Cmeter_1, 1.000},
                          {Ctrue_2, Cmeter_2, 1.000},
                          {Ctrue_3, Cmeter_3, 1.000}};

  float MAT_B[3][3]    = {{Cmeter_1_power, Ctrue_1, 1.000},
                          {Cmeter_2_power, Ctrue_2, 1.000},
                          {Cmeter_3_power, Ctrue_3, 1.000}};

  float MAT_C[3][3]    = {{Cmeter_1_power, Cmeter_1, Ctrue_1},
                          {Cmeter_2_power, Cmeter_2, Ctrue_2},
                          {Cmeter_3_power, Cmeter_3, Ctrue_3}};
                        
  //solve for coefficients "α" , "β" , "γ" by Cramer's rule
  
  float denominator = determinant3x3(main_mat);

  const float A = determinant3x3(MAT_A) / denominator;
  const float B = determinant3x3(MAT_B) / denominator;
  const float C = determinant3x3(MAT_C) / denominator;

  // Calculate Correction_factor
  const float Correction_factor = 1100.0000 / adc_chars.vref;

  //calculate Calibration Voltage
  const float C_Unadjust = Current * Correction_factor;
  const float Calibration_Current = ((A*C_Unadjust*C_Unadjust) + (B*C_Unadjust) + C ) * Manual_calibration;
  return Calibration_Current;
}
   
// ADC_pin 34
void loop()
{ 
  //!---------------------------- ----> Normal Calculation <---- ----------------------------
  float raw = Unadjusted_ADC_Read_V(ADC1_CHANNEL_6);


  //!---------------------------- ----> METHOD 2 <---- ----------------------------
  float Vdata = Linear_Calibration_voltage_Read(ADC1_CHANNEL_6, // Pin 34
                                               1.5038,  // Vtrue_1
                                               9.0000,   // Vtrue_2
                                               0.8473, // Vmeter_1
                                               8.3319, // Vmeter_2
                                               1.000 ); // manual calibrate

  // //!---------------------------- ----> METHOD 3 <---- ----------------------------
  float Vdata1 = Non_Linear_Calibration_voltage_Read(ADC1_CHANNEL_6, // Pin 34
                                             2.52,    // Vtrue_1
                                             9.988,      // Vtrue_2
                                             14.978,    // Vtrue_3
                                             1.852,   // Vmeter_1
                                             9.3042,    // Vmeter_2
                                             15.1749, // Vmeter_3
                                             1.000 ); // manual calibrate

  float raw1 = Unadjusted_ADC_Read_I(ADC1_CHANNEL_7);

  data = Linear_Calibration_Current_Read(ADC1_CHANNEL_7,
                                         1.385,  // Ctrue_1
                                         4.00,   // Ctrue_2
                                         0.6182, // Cmeter_1
                                         1.9726, // Cmeter_2
                                         1.000 ); // manual calibrate

  float data1 = Non_Linear_Calibration_Current_Read(ADC1_CHANNEL_7,
                                         0.803,    // Ctrue_1
                                         2.005,      // Ctrue_2
                                         4.089,    // Ctrue_3
                                         0.4222,   // Cmeter_1
                                         1.0445,    // Cmeter_2
                                         2.0945, // Cmeter_3
                                         1.000 ); // manual calibrate
  delay(1000);  
  Serial.println("P non-linear = " + String(Vdata1*data1, 4) + " | " + "P linear = " + String(Vdata*data, 4) + " | " + "P raw = " + String(raw*raw1, 4));

}