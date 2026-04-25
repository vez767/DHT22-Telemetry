# Low-Level STM32 Environmental Monitor: I2C LCD & 1-Wire DHT22

## Overview
This repository contains a custom hardware abstraction layer (HAL) built for the STM32 Nucleo series. The project functions as a fault-tolerant environmental data pipeline:
1. The STM32 reads raw data from a DHT22 sensor using a custom 1-Wire protocol, adhering to the manufacturer's microsecond delay timings.
2. The acquired "Current Climate" is passed through an asynchronous software fault-tolerance loop.
3. The resulting "Displayed Climate" is transmitted via I2C to a Freenove 1602 LCD.

The primary engineering goal of this module was flash memory optimization (bypassing standard C libraries), robust asynchronous hardware fault-tolerance, and implementing a flickerless UI by updating LCD memory.

## Live Demonstration
> https://github.com/user-attachments/assets/362e7d44-1b7c-4f79-811b-e2c77592eefa



**System in Action:** This demonstration highlights the completed data pipeline. The STM32 continuously polls the DHT22, processes the raw telemetry without standard C libraries, and transmits the data via I2C to the Freenove LCD. The UI was implemented in such a way that static text remains completely solid while the data coordinates surgically updates, and the asynchronous fault-tolerance loop handles disconnections without crashing the system.

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
* **Diagnosis:** It was discovered that the hardware timer (`TIM`) registers (such as limits like `ARR`, `PSC`, etc.) would not physically update their internal shadow registers to apply the new settings.
* **Solution:** Explicitly enforced the Event Generation Register (`TIM3_EGR |= 1`) to force an update to the shadow registers. 

> https://github.com/user-attachments/assets/c4dd1da3-7add-40ca-9280-393b1ea3412c



### 2. Live Logic Analyzer Verification
Before finalizing the DHT22 driver, the baseline timing logic was prototyped and verified using an Arduino. A Saleae Logic Analyzer was then used to verify the microsecond duty cycles between the DHT22 and the STM32, confirming delays were strictly adhering to the manufacturer's datasheet.

[Referenced Datasheet](https://cdn.sparkfun.com/assets/f/7/d/9/c/DHT22.pdf)

#### DHT to STM-F Communication Duty Cucles
> <img width="1358" height="314" alt="Screenshot 2026-04-13 155527" src="https://github.com/user-attachments/assets/20377381-5712-4b82-9778-53344ab99a17" />
> <img width="1401" height="319" alt="Screenshot 2026-04-13 155552" src="https://github.com/user-attachments/assets/6bbf90e5-0b2e-41ee-b9e5-4429855b85b8" />
> <img width="1245" height="232" alt="Screenshot 2026-04-13 155607" src="https://github.com/user-attachments/assets/33c0d2b8-021c-4bbf-b7c0-7f09b8764a80" />
> <img width="1386" height="307" alt="Screenshot 2026-04-13 155624" src="https://github.com/user-attachments/assets/c9f7ffae-d3d3-407b-a82e-e0d8178b2b80" />
> <img width="1241" height="255" alt="Screenshot 2026-04-13 155706" src="https://github.com/user-attachments/assets/ab5e996d-29e5-4b2c-a897-351430e426d0" />


> <img width="1249" height="264" alt="Screenshot 2026-04-13 165059" src="https://github.com/user-attachments/assets/acacfd1f-1095-422d-9b52-7fceb65a55a4" />








### 3. Floating-Point Precision & Representation Limits
During live debugging, acquired temperature and humidity values were manifesting as e.g. `27.9999989` rather than clean decimals. This is not a software bug, but a known IEEE 754 floating-point representation limitation inherent to the MCU's architecture. 
* **Resolution:** While the raw float retains this precision in live memory, it was safely handled for the UI by engineering a custom two-stage parsing wrapper (`Float_To_String`). This wrapper safely extracts only the integer and the first fractional component, ensuring the LCD only ever prints a clean, 1-decimal-place value (e.g., `28.0`).

  
 > <img width="735" height="475" alt="Screenshot 2026-04-25 160945" src="https://github.com/user-attachments/assets/ea4095ad-ffeb-4024-b354-9e2ec5543233" />

 
 > <img width="4032" height="2016" alt="meh" src="https://github.com/user-attachments/assets/9d2b9993-cf93-484d-909a-61d162182551" />



### 4. Asynchronous Fault-Tolerance & The Ghost-Character-State Machine
The system features a robust fault-tolerance loop designed to handle physical hardware disconnects. 
* **The 12-Second Fail-Safe:** If the DHT22 **Signal Wire** is physically disconnected, the system enters a fault-tolerance loop, continuously polling the hardware for approximately 12 seconds. If the sensor does not recover, the system drops a safe-state error code (`999.0`) to the display.
* **The Ghost Character Fix:** A major UI bug occurred when the sensor reconnected and resumed accurate telemetry (e.g., displaying `24.5` over the old `999.0`). The LCD retained the trailing `0` in its hardware memory, resulting in a ghost value of `24.50C`. 
* **Solution:** Instead of constantly redrawing the whole screen, a rising-edge State function was implemented (`if previous_status != current_status`). When the system recovers from a fault, it dynamically injects space characters to replace the stale memory on the glass.


> https://github.com/user-attachments/assets/a052397f-664f-48be-b05e-de7621b11d39



> https://github.com/user-attachments/assets/ef600555-bf89-4949-bc53-5aac1db04b27






## Known Limitations & Future Roadmap
* **GND Disconnect Hard-Fault:** Currently, the software fault-tolerance loop successfully catches *Signal wire* disconnects. However, physically pulling the *GND wire* causes the STM32 debugger to timeout and the `current_state` to drop to 0, essentially locking the system. This hardware-level lockup is slated to be resolved in the next architectural phase using the STM32's Independent Watchdog (IWDG) to force a silicon-level reboot.
* **Negative Number Parsing:** The custom `Int_To_String` bare-metal engine does not currently evaluate negative numbers (`-`). Error codes must be handled as positive integers or floats until advanced string manipulation is introduced.


## Acknowledgments & References
* **Adafruit DHT22 Reference:** The official [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) was referenced to understand the fundamental timing logic before migrating the concepts to bare-metal C for the STM32.
* **AI Attribution:** Google Gemini was utilized as an interactive tutor during this project. Rather than writing the application logic, the AI was strictly prompted to assign tasks, provide architectural requirements/constraints, and review my logic to foster deep comprehension of the STM32 reference manual.
