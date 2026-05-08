# Cyber-sensor-1

A comprehensive collection of ESP32 and Arduino-based sensor implementations, focusing on data acquisition, sensor interfacing, and advanced calibration techniques.

## Project Structure

This repository is organized into two primary laboratory sections:

### [LAB 1](./LAB1/) - Sensor Interfacing & Basic Implementation
Focuses on the fundamental integration of various sensors using standard libraries and basic ADC techniques.

*   **Current & Voltage:** Basic measurement using ESP32 ADC and resistor dividers.
*   **Gas Sensor (MQ135):** Air quality monitoring using the MQ135 library.
*   **Heart Rate (MAX30100):** Pulse oximetry integration via I2C.
*   **Metal Detector:** Proximity sensing using digital input.
*   **Thermocouple (MAX6675):** High-temperature measurement via bit-banged SPI.
*   **Waterproof Temp (DS18B20):** Precise temperature sensing using 1-Wire protocol.

### [LAB 2](./LAB2/) - Advanced Calibration & Optimization
Focuses on improving accuracy through mathematical modeling and library-independent implementations.

*   **Quadratic Calibration:** Implementation of second-order polynomial fitting ($y = ax^2 + bx + c$) using Cramer's Rule and 3x3 determinants for Voltage, Current, and Power calibration.
*   **MQ135 Non-Library:** Interfacing with gas sensors using direct ADC characterization and manual calculations.
*   **Multi-Point Calibration:** Scripts to calibrate sensors against reference meters using linear and non-linear regression.

---

## Technical Specifications

### Typical Pin Mapping (ESP32)

| Sensor / Module | Pin Name | GPIO / Channel | Protocol |
| :--- | :--- | :--- | :--- |
| **MQ135 Gas Sensor** | MQ_PIN | GPIO 34 (ADC1_CH6) | Analog |
| **Voltage Sensor** | ADC_pin | GPIO 34 (ADC1_CH6) | Analog |
| **Current Sensor** | ADC2_pin | GPIO 35 (ADC1_CH7) | Analog |
| **Metal Detector** | sensor | GPIO 32 | Digital In |
| **Status LED** | LED | GPIO 33 | Digital Out |
| **DS18B20 Temp** | ONE_WIRE_BUS | GPIO 22 | 1-Wire |
| **MAX6675 (Data)** | dataPin | GPIO 7 | SPI (Soft) |
| **MAX6675 (Clock)** | clockPin | GPIO 6 | SPI (Soft) |
| **MAX6675 (Select)** | selectPin | GPIO 5 | SPI (Soft) |
| **MAX30100** | SDA/SCL | GPIO 21 / 22 | I2C |

### Calibration Logic
In `LAB2`, the project implements a robust calibration system:
1.  **Sampling:** Averages 100 samples per reading to reduce high-frequency noise.
2.  **ADC Characterization:** Uses `esp_adc_cal` to account for ESP32's internal Vref variations.
3.  **Polynomial Regression:** Solves a system of equations for three calibration points $(V_{meter}, V_{true})$ to find coefficients $\alpha, \beta, \gamma$.
4.  **Cramer's Rule:** Used to solve the $3 \times 3$ matrix determinant for quadratic correction.

## Hardware Requirements

*   **Microcontroller:** ESP32 (strongly recommended for Lab 2 features).
*   **Modules:** MQ135, MAX30100, MAX6675, DS18B20, ACS712 (or similar current sensor).

## Getting Started

1.  **Library Setup:**
    The required libraries are included locally within the subdirectories. If using the Arduino IDE, ensure the library folders (e.g., `MAX6675`, `MQ135`) are in your include path or moved to your `Documents/Arduino/libraries` folder.
2.  **ESP32 Core:**
    Install the ESP32 board support via the Arduino Boards Manager: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`.
3.  **Calibration Process:**
    To use the calibration scripts in Lab 2, you will need a reliable multimeter to provide the "True" reference values for the multi-point calibration.

## Features
*   **High Precision:** 12-bit ADC utilization with software-based oversampling.
*   **Mathematical Modeling:** Real-time solving of calibration matrices on-chip.
*   **Modular Code:** Clean separation between raw data acquisition and calibrated output.
