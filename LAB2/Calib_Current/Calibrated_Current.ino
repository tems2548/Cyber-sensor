#include <Arduino.h>

//--------------include lib from datasheet--------------------//
#include <driver/adc.h>
#include <esp_adc_cal.h>
//------------------------------------------------------------//

// define ADC pin
#define ADC_pin 34
#define ADC2_pin 35

bool DEBUG = true;

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
  pinMode(ADC_pin, INPUT);
  pinMode(ADC2_pin, INPUT);
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
  float raw = Unadjusted_ADC_Read_I(ADC1_CHANNEL_7);
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
  Serial.println("Calibration_Current_non = " + String(data1, 4) +" | "+ "Calibration_Current = " + String(data, 4) + " | " + "Uncalibration_Current = " + String(raw, 4));
}
