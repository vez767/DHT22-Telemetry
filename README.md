# Low-Level STM32 Environmental Monitor: FreeRTOS Architecture

> **Note:** For the legacy baseline, HIL (Hardware-in-the-Loop) debugging videos, and the foundational 1-Wire timing logic, please refer to [ARCHITECTURE.md](ARCHITECTURE.md).

## Overview
This repository contains a custom Hardware Abstraction Layer (HAL) for the STM32 Nucleo-F401RE, engineered specifically to bridge the gap between microscopic hardware protocols and a Real-Time Operating System (FreeRTOS). 

The system reads raw data from a DHT22 sensor, enforces hardware constraints, processes the payload safely through a thread-safe Queue, and renders it to a Freenove I2C 1602 LCD module without causing system hangs or data corruption.

## Architecture & Key Features

### 1. The Preemption Shield (DHT22)
The DHT22 communicates using a clockless, half-duplex single-wire protocol where digital `0`s and `1`s are differentiated purely by the microsecond width of the voltage pulse (28µs vs 70µs). 
* **The RTOS Conflict:** The FreeRTOS SysTick hardware timer fires every 1 millisecond. If the OS preempts the CPU mid-pulse to run the scheduler, the pulse timing is lost, and the 40-bit packet is permanently corrupted.
* **Engineering Solution:** The data acquisition phase is wrapped in a preemption shield (`taskENTER_CRITICAL()`). This physically disables OS interrupts for the required time to catch the payload before instantly restoring the OS.

### 2. Hybrid Delay Systems 
To maximize CPU availability, this driver utilizes a hybrid timing architecture:
* **Macro-Delays (OS Yielding):** The 1ms+ initialization wake-up pulses utilize `vTaskDelay()`, handing the CPU back to the kernel so parallel system tasks can execute.
* **Micro-Delays (Hardware Polling):** Because FreeRTOS cannot yield in increments smaller than 1ms, a dedicated Basic Timer (`TIM3`) is configured to act as a 1MHz stopwatch. This handles the microscopic 2µs and 80µs handshakes required by the manufacturer's silicon.

### 3. Hysteresis (Deadband) UI Filter
* **Requirement:** Optimize I2C bus utilization and prevent visual UI flickering.
* **Constraint:** The physical I2C bus operates at 100kHz, creating a massive CPU bottleneck compared to the 16MHz STM32 HSI core.
* **The Action:** Implemented a mathematical Flag pattern within the consumer task. The MCU evaluates the telemetry payload in RAM and strictly bypasses the I2C peripheral unless the ambient environment shifts by `0.5°C` (Temperature) or `1.0%` (Humidity).

### 4. Hardware Thermal Constraints
The DHT22 features an internal thermistor that is highly susceptible to self-heating. `vClimateTask` enforces a strict hardware limit by utilizing `vTaskDelay(pdMS_TO_TICKS(2000))` to guarantee a mandatory 2-second thermal cooldown between physical reads.

---

## Hardware Integration & HIL Debugging

The transition from a bare-metal low level architecture to an RTOS environment introduced strict timing tolerances. Hardware-in-the-Loop (HIL) analysis was required to synchronize the FreeRTOS scheduler with the physical silicon.

### I2C Bus Verification & The Blank Screen Bug
During the initial RTOS port, the Freenove LCD failed to initialize, resulting in a completely blank screen. A Saleae Logic Analyzer was deployed to verify the physical layer. 

*The logic analyzer confirmed that the pulse widths (PW) were correct and communication between the MCU and the I2C backpack was successfully established. The inital suspicion was that the bug was purely a timing granularity issue. For hardware handshakes that needed exactly 1000µs (1ms), we initially used `vTaskDelay(pdMS_TO_TICKS(1))`. However, because the FreeRTOS SysTick can fire asynchronously, the 1 tick sometimes may be executed too quickly for the LCD to finish booting. By increasing these macro-delays slightly (e.g., from 1 tick to 3 ticks), the hardware was enough guaranteed leeway to process the initialization handshake fully.*

> <img src="https://github.com/user-attachments/assets/8a760576-8161-4b2a-907e-4da46d9fb256" />

*Caption: Logic analyzer capturing the clock and data lines, confirming active MCU communication despite the initial blank display.*

> https://github.com/user-attachments/assets/336cceb8-dee9-4b56-b6a5-9e54b3c62aaf





### Successful Telemetry Rendering
Following the delay timing fixes and the integration of `xClimateQueue`, the `vDisplayTask` successfully acts as a consumer, safely receiving and drawing dynamic payload data to the glass.

The accompanying video demonstrates the LCD operating nominally, followed by a simulated hardware failure (a severed data wire). The system successfully catches the fault and triggers a Safe State fallback, overriding the UI to render diagnostic error codes (`999.0` for both `Temp` and `Hum`) to prevent the display of corrupted or stale data.



> https://github.com/user-attachments/assets/1ebb6bc6-8964-46de-9104-44046a0bcfc6



---

## Logged Hardware Failures & Limitations

The hardware anomalies and driver limitations encountered during development are actively documented for future refactoring.

### The 4-Bit Multiplexer Desynchronization (Warm Boot Bug)
The HD44780 controller requires instructions to be split into upper and lower 4-bit nibbles. If the STM32 undergoes a warm reset (via the physical black reset button on the Nucleo) precisely mid-transmission, the LCD retains power but loses state synchronization.

* **Symptom 1 - Nibble Misalignment:** The LCD grabs the first boot command from the MCU, assumes it is the missing lower nibble of the interrupted command, and permanently shifts all subsequent bytes by 4 bits, resulting in garbage characters.



> https://github.com/user-attachments/assets/690307a3-4ea5-4085-aea6-96da99687bcc



* **Symptom 2 - Complete State Lockout:** In very rare severe cases, the state machine becomes entirely trapped in 4-bit mode and refuses all subsequent 8-bit mode wake-up commands, resulting in a permanently blank screen regardless of multpile MCU reboot attempts.

> https://github.com/user-attachments/assets/53248cca-e0a0-4934-a421-f369ae10fe6d






* **Current Workaround:** A cold boot (hard power cycle via USB disconnect) is utilized to drain the hardware capacitors and forcefully reset the HD44780 internal state machine. **Note:** Even with a cold boot, the screen is sometimes still susceptible to the 4-bit mode lockup, requiring occasional subsequent resets.
* **Future Action (Tier 2.5/3):** Implement a dedicated raw-nibble software reset sequence inside `LCD_Init()` to bypass the multiplexer and bulletproof the driver against MCU reboots.

---

## Hardware Setup & Pinout

**Microcontroller:** STM32 Nucleo-F401RE
**Peripherals:** DHT22 Sensor, Freenove I2C 1602 Module (HD44780)

| Peripheral Module | Device Pin | STM32 Pin | Description / Notes |
| :--- | :--- | :--- | :--- |
| **DHT22** | VCC | 3.3V | Main power for the temperature/humidity sensor. |
| **DHT22** | GND | GND | Common ground. |
| **DHT22** | SIG | PA0 | Data wire. **Must be bridged with a 10kΩ pull-up resistor to 3.3V.** |
| **Freenove LCD** | GND | GND | Common ground. |
| **Freenove LCD** | VCC | 5.0V | **Required:** The I2C backpack logic operates strictly on the 5V rail. |
| **Freenove LCD**| SDA | PB9 | I2C Data Line (Open Drain, Alternate Function 4). |
| **Freenove LCD**| SCL | PB8 | I2C Clock Line (Open Drain, Alternate Function 4). |

---

## Future Roadmap (Current Phase: Inter-Task Telemetry)

### Upcoming (Tier 2)
* **Resource Management (Mutexes):** Integration of an I2C MPU-6050 accelerometer. A FreeRTOS Mutex key will be engineered to manage bus contention and prevent collisions between the LCD consumer task and the MPU-6050 sensor task.
* **Independent Watchdog (IWDG) Fault Recovery:** While the system currently handles data-wire disconnects via software Safe States, physically severing the ground wire causes a hardware lockup that stalls the RTOS. An Independent Watchdog (IWDG) timer will be reinstated into the architecture to autonomously hard-reset the MCU and recover the system if a pulled ground wire causes the scheduler to freeze.

### Pending Hardware Integration (Suspended)
* **Direct Memory Access (DMA):** An initial attempt was made to migrate I2C transmissions to DMA to reduce CPU blocking. While the I2C hardware handshake was successfully established, a synchronization issue caused the DMA controller to stream corrupted/garbage payload data. This experimental DMA logic was intentionally withheld from the mainline branch to preserve current system stability, but it remains a targeted milestone for future low-power optimization.

## Acknowledgements
**AI Attribution:** Google Gemini was utilized as an interactive engineering tutor during this project. Rather than writing the application logic, the AI was strictly prompted to assign tasks, define architectural constraints, and review my logic. This methodology fostered a deep comprehension of bridging bare-metal hardware protocols (DHT22, I2C) with a Real-Time Operating System (FreeRTOS), specifically focusing on task synchronization, thread-safe queues, and resolving microsecond timing conflicts between the OS scheduler and physical silicon.
