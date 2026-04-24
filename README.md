# Low-Level STM32 Environmental Monitor: I2C LCD & 1-Wire DHT22

## Overview
This repository contains a custom, hardware abstraction layer (HAL) built for the STM32 Nucleo series. It successfully implements two distinct communication protocols from scratch:
1. **Custom 1-Wire Protocol:** To interface with the DHT22 Temperature/Humidity sensor.
2. **I2C Protocol:** To drive an HD44780 LCD display (via an I2C backpack) without relying on high-level standard libraries.

The primary engineering goal of this module was flash memory optimization and robust asynchronous fault-tolerance.

## Hardware Setup & Pinout
**Microcontroller:** STM32 Nucleo-F401RE

### 1. DHT22 Temperature & Humidity Sensor
      
| DHT22 Pin | STM32 / Nucleo Pin | Description / Notes |
| :--- | :--- | :--- |
| **VCC** | `3.3V` | Main power for the sensor. |
| **GND** | `GND` | Common ground. |
| **SIG** | `A0`  | Signal wire.  |

**SIG must be bridged with a 10kΩ resistor to 3.3V to act as a pull-up resistor.**

### 2. I2C LCD (HD44780 Controller)
*Note: The LCD logic requires the 5.0V rail to power the screen backlight and controller.*

| LCD (I2C Backpack) | STM32 / Nucleo Pin | Description / Notes |
| :--- | :--- | :--- |
| **VDD / VCC** | `5.0V` | Main power for screen and I2C module. |
| **GND** | `GND` | Common ground. |
| **SDA** | `PB9` (Header - D14/SDA) | I2C Serial Data. |
| **SCL** | `PB8` (Header -  D15/SCL) | I2C Serial Clock. |

*(The specific LCD I2C address was verified on the bench via a custom bare-metal bus scanner as `0x27`.)*

*Note: 3.3V was used for the DHT22 because the LCD required the 5V rail.*

## Architecture & Key Features
* **Decoupled HAL Architecture:** Strict separation between the generic peripheral drivers (`i2c_lcd.c`, `dht_22.c`) and the application layer (`main.c`).
* **Memory-Safe String Parsing:** Standard C libraries like `<stdio.h>` (used for `sprintf`) and `<stdlib.h>` (often used for `atoi`, `atof`, and `abs`) were intentionally bypassed. Instead, custom parsing engines (`Int_To_String`, `Float_To_String`) and manual mathematical logic were engineered. This approach drastically reduces the flash memory footprint and prevents stack bloat.
* **Flicker-Free UI Rendering:** By updating specific LCD coordinates (`LCD_Set_Cursor`) instead of issuing global clear commands, the static UI remains rock-solid during live data updates. Custom `Int_To_String` and `Float_To_String` engines were engineered to parse telemetry data securely.


## Engineering Challenges & Hardware-in-the-Loop (HIL) Debugging

### 1. The `TIM3_EGR` Shadow Register Bug
Initially, the sensor data was wildly flickering between states `1` (Success) and `-10` (Timeout). 
* **Diagnosis:** Discovered that the hardware timer (`TIM`) registers (such as limits like `ARR`, `PSC`, etc.) would not physically update their internal shadow registers to apply the new settings.
* **Solution:** Explicitly enforced the Event Generation Register (`TIM3_EGR |= 1`) to force an update to the shadow registers. 
* *[Insert Video/GIF here showing the flickering state bug]*

### 2. Live Logic Analyzer Verification
Before finalizing the DHT22 driver, the baseline timing logic was prototyped and verified using an Arduino. A Saleae Logic Analyzer was then used to verify the microsecond duty cycles between the DHT22 and the STM32, confirming our bare-metal delays were strictly adhering to the manufacturer's datasheet.

Referenced Datasheet - (https://cdn.sparkfun.com/assets/f/7/d/9/c/DHT22.pdf)
* *[Insert Logic Analyzer Screenshot here showing the duty cycles]*

### 3. Floating-Point Precision & Representation Limits
During live debugging, acquired temperature and humidity values were manifesting as e.g. `27.9999989` rather than clean decimals. This is not a software bug, but a known IEEE 754 floating-point representation limitation inherent to the MCU's architecture. 
* **Resolution:** While the raw float retains this precision in live memory, it was safely handled for the UI by engineering a custom two-stage parsing wrapper (`Float_To_String`). This wrapper safely extracts only the integer and the first fractional component, ensuring the LCD only ever prints a clean, 1-decimal-place value (e.g., `28.0`).
* *[Insert Image/Video here of the live debugger showing the 27.9999 float bug]*

### 4. Ghost Character State Machine
A major UI bug occurred when the sensor disconnected (displaying error code `999.0`) and then reconnected (displaying `24.5`). The LCD retained the trailing `0` in its hardware memory, resulting in a ghost value of `24.50C`.
* **Solution:** Instead of constantly redrawing the whole screen  a rising-edge State function was implemented (`if previous_status != current_status`). When the system recovers from a fault, it dynamically injects space characters to replace the stale memory on the glass.
* *[Insert Video here showing the ghost value appearing, and then another showing the state-machine fix]*

## Acknowledgments & References
* **Adafruit DHT22 Reference:** The official [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) was referenced to understand the fundamental timing logic before migrating the concepts to bare-metal C for the STM32.
* **AI Attribution:** Google Gemini was utilized as an interactive tutor during this project. Rather than writing the application logic, the AI was strictly prompted to assign tasks, provide architectural requirements/constraints, and review my logic to foster deep comprehension of the STM32 reference manual.
