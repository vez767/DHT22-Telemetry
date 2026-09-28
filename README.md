# Low-Level STM32 Environmental & Spatial Telemetry Monitor: FreeRTOS Architecture (v3.0)

*Note: For the legacy baseline, early HIL (Hardware-in-the-Loop) debugging videos, and foundational 1-Wire timing logic, please refer to [ARCHITECTURE.md](ARCHITECTURE.md).

## Overview

This repository contains a custom Hardware Abstraction Layer (HAL) and Real-Time Operating System (FreeRTOS) architecture for the STM32 Nucleo-F401RE. It serves as a fully independent, fault-tolerant telemetry system that concurrently reads and measures distance, ambient temperature/humidity, and 3D spatial kinematics. 

The system actively monitors:
*   **Environmental Data:** Temperature and Humidity via a DHT22 (1-Wire).
*   **Spatial Kinematics:** 3D X/Y/Z orientation via an MPU-6050 IMU (I2C).
*   **Distance/Proximity:** Centimeter-accurate ranging via an HC-SR04 Ultrasonic Sensor (Timer Input Capture).
*   **Geospatial Tracking:** NMEA coordinate parsing via a NEO-6M GPS Module (UART DMA).

Rather than relying on pre-built Arduino-style libraries, all drivers were architected from scratch using manufacturer datasheets and direct memory-mapped register access. The firmware relies heavily on FreeRTOS IPC (Inter-Process Communication) queues, hardware timers, and Mutexes to render data across **three separate I2C LCD modules** simultaneously without bus collisions or thread starvation.

## Architecture & Key Features

### 1. ISO 14971 Safe States & Fault Isolation
A core requirement of this architecture was preventing silent crashes. Because the RTOS handles each sensor in an independent task, the system features complete fault isolation:
*   **Sensor Hardware Disconnects:** If the DHT22 data wire is severed, the system catches the timeout and overrides the UI with a diagnostic `999.0` error code. If the HC-SR04 Trigger or Echo wires are pulled, the UI safely falls back to an `ERR` state.
*   **Task Independence:** Severing one sensor does not lock the I2C bus or freeze the RTOS scheduler. The other sensors and LCDs will continue rendering live data without interruption.

### 2. The HC-SR04 Anomaly Filter & State Machine
Raw ultrasonic data is highly susceptible to noise and acoustic absorption. The `vHCSR04_Task` implements a 10-sample running-average filter and a dedicated status payload to isolate faults:
*   **Status 1 (Normal):** Displays valid physical distance.
*   **Status -1 (Target Lost):** Triggered if an object absorbs the ultrasonic ping (e.g., a sponge) or moves out of the ~6.5m maximum range, forcing a safe buffer flush.
*   **Status -2 (Wire Fault):** Triggered via a 50ms RTOS timeout if a physical wire is severed.

### 3. Resource Synchronization (Mutex I2C Arbitration)
**Requirement:** Prevent I2C bus collisions between the MPU-6050 IMU and the three HD44780 LCD controllers.
**Action:** Deployed a FreeRTOS Mutex (`xI2C1_Mutex`). This guarantees thread-safe bus sharing, ensuring that the background 500ms motion-polling task cannot interrupt or corrupt the high-frequency UI rendering task.

### 4. Hybrid Delay Systems & Preemption Shields
*   **Preemption Shield:** The DHT22's 40-bit payload requires microsecond-perfect timing. `taskENTER_CRITICAL()` physically disables OS interrupts during the read sequence so the 1ms FreeRTOS SysTick does not corrupt the payload.
*   **Timer Input Capture:** The HC-SR04 utilizes TIM2 configured in Input Capture mode, triggering interrupts on rising/falling edges to calculate precise microsecond durations without blocking the CPU in empty `while()` loops.

### 5. Memory Diagnostics (Stack Watermarking)
To mathematically guarantee the UI task will not trigger a stack overflow during complex float-to-string math across three displays, the stack margin is actively tracked via the `display_watermark` variable using `uxTaskGetStackHighWaterMark`. Additionally, a `vApplicationStackOverflowHook` is implemented to trap the CPU in a deterministic safe-state loop if any task breaches its memory threshold
> <img width="750" height="178" alt="Screenshot 2026-07-02 175628" src="https://github.com/user-attachments/assets/843de20e-a5e3-4582-aa1c-deb72a44124f" />

#### *FreeRTOS High Watermark validation confirming safe memory margins under maximum system load.*

### 6. Asynchronous GPS Ingestion (UART DMA)
To prevent NMEA string parsing from blocking the CPU, the NEO-6M GPS communicates via USART6 utilizing a hardware-level Circular DMA Buffer. The RTOS task ingests geospatial coordinates in the background, guaranteeing that satellite tracking does not steal clock cycles from the high-frequency I2C LCD rendering task.

<img width="674" height="46" alt="Screenshot 2026-09-14 134336" src="https://github.com/user-attachments/assets/45898ba8-f814-4834-b87f-3f0d73fab276" />


---
## Hardware Integration & HIL Debugging

The transition to a multi-node RTOS architecture required strict Hardware-in-the-Loop (HIL) verification. 

### Multi-Display I2C Addressing
To drive three identical Freenove 1602 LCDs on a single I2C bus, the PCF8574 backpack addresses were hardware-modified via soldering:
*   **LCD 1 (Environment - DHT22):** `0x27` (Default - All pads open)
*   **LCD 2 (Navigation - MPU-6050):** `0x26` (Pad A0 bridged to Ground)
*   **LCD 3 (Distance - HC-SR04):** `0x23` (Pad A2 bridged to Ground)


<img src="https://github.com/user-attachments/assets/0ad1764b-52c0-4a39-99b4-7479e9881378" width="32%" />


<img src="https://github.com/user-attachments/assets/ff759a5e-1678-4a8a-85c7-5c63e3fcf9aa" width="32%" />


<img src="https://github.com/user-attachments/assets/65382c44-5eae-4fd6-8053-fb59128ff4a9" width="32%" />




### HIL Verification Showcases
* **Full System Telemetry & Task Independence:** All three screens render data smoothly without bus collisions. Includes active NEO-6M geospatial tracking verified simultaneously via UART output on a PuTTY terminal.

> <video src= "https://github.com/user-attachments/assets/c85f6cc6-1672-4fde-892f-f4b193c6d4dd"  width="600" controls></video>




  
* **Fault Isolation (DHT22):** Physically severing the DHT22 `SIG` wire (Pin `PA0`) triggers the `999.0` error code on the Environment LCD, while the Gyro and Distance LCDs continue updating  in real-time.
>  <video src= "https://github.com/user-attachments/assets/108a37f1-de67-48c1-b262-84feaec79e70" width="500" controls></video>



* **HC-SR04 Fault Isolation & Acoustic Limits:** Demonstrates the 3-tier state machine. A sponge is used to absorb the acoustic ping, triggering the `-1 (TRGT LOST)` hardware limitation and dumping stale buffer data. Finally, the `TRIG` wire (`D8/PA9`) is physically severed, instantly tripping the RTOS timeout and locking the screen into a `-2 (WIRE FAULT)` safe state whilst the Gyro and Temperature LCDs continue updating  in real-time.

>  <video src= "https://github.com/user-attachments/assets/e162aba9-1aeb-4a52-a3e9-c2649a8b1dc4" width="500" controls></video>








---

## Logged Hardware  Limitations

### Multi-LCD Desynchronization (Warm Boot Bug)
The HD44780 controller requires instructions to be split into upper and lower 4-bit nibbles. If the STM32 undergoes a cold boot (hard power cycle via USB), the three LCD multiplexers often fail to synchronize simultaneously.
*   **Symptom:** Only 1 or 2 of the 3 screens will successfully initialize; the others remain blank or display garbage characters due to nibble misalignment.

> <video src="https://github.com/user-attachments/assets/185b82fc-364e-46ff-bdc7-b904437d71ab" width="500" controls></video>

*   **Current Workaround:** Multiple warm boots (via the STM32 reset button) are required to reset the HD44780 internal state machines.
*   
### Geospatial Cold Start 
*   **Cold Start Lock:** The NEO-6M requires a direct line of sight to the sky. Initial lock (cold start) can take several minutes, during which the system will report un-fixed baseline coordinates.




---

## Hardware Setup & Pinout

**Microcontroller:** STM32 Nucleo-F401RE

| Peripheral | Device Pin | STM32 Pin | Description / Notes |
| :--- | :--- | :--- | :--- |
| **DHT22** | SIG | `PA0` | 1-Wire Data. 10kΩ pull-up resistor to 3.3V required. |
| **HC-SR04** | TRIG | `PA9 / D8` | Standard GPIO Output. |
| **HC-SR04** | ECHO | `PA1` | `TIM2_CH2` Input Capture (Alternate Function). |
| **I2C Bus 1** | SCL | `PB8` | Shared Clock for MPU-6050 and 3x LCDs (Open Drain). |
| **I2C Bus 1** | SDA | `PB9` | Shared Data for MPU-6050 and 3x LCDs (Open Drain). |
| **Power** | VCC | 5V / 3.3V | LCDs require 5V rail; Sensors operate on 3.3V rail. |
| **NEO-6M** | TX | `PA12` | USART6 RX (DMA Circular Buffer). |
| **NEO-6M** | VCC | 5V | Requires 5V rail for satellite lock. |
| **NEO-6M** | GND | GND | Common ground. |

---

## Future Roadmap & Upcoming Modules

As the curriculum moves toward advanced firmware topics, the following hardware constraints will be addressed:
*   **Independent Watchdog (IWDG) Fault Recovery:** While the software currently handles disconnected data wires, physically severing a sensor's *ground* wire causes a hardware lockup that stalls the RTOS. An IWDG timer will be implemented to autonomously hard-reset the MCU if the scheduler freezes.
*   **Geospatial UI Integration:** While NEO-6M coordinates are currently routed to a terminal via UART, the next UI iteration will integrate Latitude/Longitude payloads directly into the I2C LCD matrix.

  
## Acknowledgements
**AI Attribution:** Google Gemini was utilized as an interactive engineering tutor during this project. Rather than writing the application logic, the AI was strictly prompted to assign tasks, define architectural constraints, explain register-level logic, and review code. This methodology fostered a deep comprehension of bridging bare-metal hardware protocols with a Real-Time Operating System.
