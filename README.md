# Low-Level STM32 Environmental Monitor: FreeRTOS Architecture



*Note: For the legacy baseline, HIL (Hardware-in-the-Loop) debugging videos, and the foundational 1-Wire timing logic, please refer to [ARCHITECTURE.md](ARCHITECTURE.md).*



## Overview



This repository contains a custom Hardware Abstraction Layer (HAL) for the STM32 Nucleo-F401RE. It was engineered as a hands-on educational project to deeply understand how to bridge the gap between microscopic hardware protocols and a Real-Time Operating System (FreeRTOS). 



Serving as the foundational architecture for a bio-adaptive monitor, the system reads raw data from a DHT22 sensor and 3D spatial kinematics from an MPU-6050 IMU. Rather than relying on pre-built libraries, the [MPU-6050 Register Map and Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf) were actively consulted to architect the MPU-6050 driver and its corresponding I2C protocol from scratch. 



The resulting firmware enforces hardware constraints, synchronizes shared resources via Mutexes, processes payloads safely through thread-safe Queues, and renders a dual-dashboard interface to a Freenove I2C 1602 LCD module without causing system hangs or data corruption.



---



## Architecture & Key Features



### 1. The Preemption Shield (DHT22)

The DHT22 communicates using a clockless, half-duplex single-wire protocol where digital 0s and 1s are differentiated purely by the microsecond width of the voltage pulse (28µs vs 70µs). (For logic analyzer captures verifying these pulse widths, please refer to `ARCHITECTURE.md`).

* **The RTOS Conflict:** The FreeRTOS SysTick hardware timer fires every 1 millisecond. If the OS preempts the CPU mid-pulse to run the scheduler, the pulse timing is lost, and the 40-bit packet is permanently corrupted.

* **Engineering Solution:** The data acquisition phase is wrapped in a preemption shield (`taskENTER_CRITICAL()`). This physically disables OS interrupts for the required time to catch the payload before instantly restoring the OS.



### 2. Hybrid Delay Systems

To maximize CPU availability, the dht-22 driver utilizes a hybrid timing architecture:

* **Macro-Delays (OS Yielding):** The 1ms+ initialization wake-up pulses utilize `vTaskDelay()`, handing the CPU back to the kernel so parallel system tasks can execute. Crucially, for critical hardware setup steps requiring a 1ms window (such as the LCD boot sequence), the macro-delay was increased from 1 tick to 3 ticks (`vTaskDelay(3)`). Because the FreeRTOS SysTick operates asynchronously to task execution, a 1-tick delay can expire almost instantly if called right before a timer interrupt, failing to satisfy the silicon's minimum timing requirement.

* **Micro-Delays (Hardware Polling):** Because FreeRTOS cannot yield in increments smaller than 1ms, a dedicated Basic Timer (TIM3) is configured to act as a 1MHz stopwatch. This handles the microscopic 2µs and 80µs handshakes required by the manufacturer's silicon without causing OS tick distortion.



### 3. Resource Synchronization (Mutexes)

* **Requirement:** Prevent I2C bus collisions between the MPU-6050 IMU and the HD44780 LCD controller.

* **The Action:** Deployed a FreeRTOS Mutex (`xI2C1_Mutex`). This guarantees thread-safe bus sharing, ensuring that the background motion-polling task cannot interrupt or corrupt the active UI rendering task.



### 4. Memory Diagnostics & Safety Hooks

* **Requirement:** Prevent silent system crashes due to UI dynamic memory allocation.

* **The Action:** Instrumented the UI task with FreeRTOS Stack Watermarking (`uxTaskGetStackHighWaterMark`). Additionally, implemented `vApplicationStackOverflowHook` as a fail-safe to trap the CPU in a deterministic safe-state loop if any task breaches its memory threshold.



### 5. Hysteresis (Deadband) UI Filter

* **Requirement:** Optimize I2C bus utilization and prevent visual UI flickering.

* **Constraint:** The physical I2C bus operates at 100kHz, creating a massive CPU bottleneck compared to the 16MHz STM32 HSI core.

* **The Action:** Implemented a mathematical Flag pattern within the consumer task. The MCU evaluates the telemetry payload in RAM and strictly bypasses the I2C peripheral unless the ambient environment shifts by 0.5°C (Temperature) or 1.0% (Humidity), or the MPU-6050 breaches its `+/- 70` noise floor deadband.



### 6. Hardware Thermal Constraints

The DHT22 features an internal thermistor that is highly susceptible to self-heating. `vClimateTask` enforces a strict hardware limit by utilizing `vTaskDelay(pdMS_TO_TICKS(2000))` to guarantee a mandatory 2-second thermal cooldown between physical reads.



---



## Hardware Integration & HIL Debugging



The transition from a bare-metal low-level architecture to an RTOS environment introduced strict timing tolerances. Hardware-in-the-Loop (HIL) analysis was required to synchronize the FreeRTOS scheduler with the physical silicon.



### Pre-Flight Hardware Verification

Before implementing the bare-metal C drivers, the MPU-6050 module required physical pin soldering. To ensure the silicon was not Dead on Arrival, the sensor was soldered and validated on a known-good stack (Arduino + [FastAArduino.Mngles](https://github.com/joaoaugustocz/mpu6050_FastAngles) library before integration into the STM32 architecture.



|  MPU-6050 Soldered | LED Light On After Pin Connection |
| :---: | :---: |
| <img src="https://github.com/user-attachments/assets/040d2806-bb58-40c8-8086-8e05d635e05d" width="45%"/> | <img src="https://github.com/user-attachments/assets/2cf13fdb-4605-4391-89ac-6bab20ff8512" width="45%"/> |






https://github.com/user-attachments/assets/d25d3976-5147-4f3f-8aca-7a5d9ad9b89a




### I2C Bus Verification & The Blank Screen Bug

During the initial RTOS port, the Freenove LCD failed to initialize, resulting in a completely blank screen. A Saleae Logic Analyzer was deployed to verify the physical layer.

The logic analyzer confirmed that the pulse widths (PW) were correct and communication between the MCU and the I2C backpack was successfully established. The initial suspicion was that the bug was purely a timing granularity issue. For hardware handshakes that needed exactly 1000µs (1ms), we initially used `vTaskDelay(pdMS_TO_TICKS(1))`. However, because the FreeRTOS SysTick can fire asynchronously, the 1 tick sometimes may be executed too quickly for the LCD to finish booting. By increasing these macro-delays slightly (e.g., from 1 tick to 3 ticks), the hardware was guaranteed enough leeway to process the initialization handshake fully.



*Caption: Logic analyzer capturing the clock and data lines, confirming active MCU communication despite the initial blank display.*

> https://github.com/user-attachments/assets/336cceb8-dee9-4b56-b6a5-9e54b3c62aaf



### The Supply Chain Quirk (0x70 vs 0x68)

During the bare-metal I2C register configuration, a low-level `WHO_AM_I` identity check was programmed. While the MPU-6050 datasheet explicitly states the module should return `0x68`, the debugger captured a return value of `112` which is `0x70` in Hexadecimal. This confirmed a common supply-chain anomaly: the manufacturer had actually soldered an upgraded **MPU-6500** silicon die onto an MPU-6050 breakout board. 

<table>
  <tr>
    <th colspan="2">MPU-6050 Board Returning MPU-6500 WHO_AM_I Value (0x70 / 112)</th>
  </tr>
  <tr>
    <td align="center" valign="middle">
      <img src="https://github.com/user-attachments/assets/425f3bea-b3a3-450b-82c4-ed68203acc00" height="220" style="object-fit: contain;" alt="Debugger WHO_AM_I value 112">
    </td>
    <td align="center" valign="middle">
      <img src="https://github.com/user-attachments/assets/e4a73bcb-0a87-41be-b124-2de0afb95593"<img width="380" height="68" alt="Screenshot 2026-07-02 175731" src="https://github.com/user-attachments/assets/8c63561c-ffc7-46f1-b2a8-e699f053df26" />
 height="220" style="object-fit: contain;" alt="Zoomed debugger view">
    </td>
  </tr>
</table>

 





### Stack Watermarking Proof

To mathematically guarantee the UI task would not trigger a stack overflow during complex float-to-string math, the FreeRTOS High Watermark was monitored. At an idle state, the RTOS reported **166 Words** remaining. Under high-frequency physical articulation (forcing maximum screen redraws and math operations), the watermark dropped to **124 Words**, proving a completely safe memory margin under maximum system load.

<img src="https://github.com/user-attachments/assets/2d167e12-c13a-43fc-82b5-0eb8ce25e9b4" />





### Successful Telemetry Rendering

Following the delay timing fixes and the integration of the IPC Queues, the `vDisplayTask` successfully acts as a consumer, safely unpacking structs to render a dual-dashboard layout (Row 0: Temp/Humidity, Row 1: X/Y/Z spatial data).



The system includes a hardware fail-safe: if the DHT22 data wire is physically severed, the system catches the fault and triggers a Safe State fallback, overriding the UI to render diagnostic error codes (`999.0` for Temp/Hum) to prevent the display of corrupted or stale data.



https://github.com/user-attachments/assets/a751ba67-611f-4c03-a561-46f3284c4742





---



## Logged Hardware Failures & Limitations



### UI Constraint: Rapid Articulation Ghosting

* **Symptom:** Rapid physical articulation of the MPU-6050 sensor causes temporary character bleeding and ghosting on the LCD. 

* **Cause:** Simultaneous large spatial values temporarily exceed the physical 16-character limit of the screen. The artifacts remain until physical motion stops and the mathematical deadband filter resets the display to `+0`.



https://github.com/user-attachments/assets/5b4393c6-3cff-4549-b25d-a96ef8c67edc





### The 4-Bit Multiplexer Desynchronization (Warm Boot Bug)

The HD44780 controller requires instructions to be split into upper and lower 4-bit nibbles. If the STM32 undergoes a cold boot ( via hard power cycle via USB disconnect) precisely mid-transmission, the LCD loses state synchronization.

* **Symptom 1 - Nibble Misalignment:** The LCD grabs the first boot command from the MCU, assumes it is the missing lower nibble of the interrupted command, and permanently shifts all subsequent bytes by 4 bits, resulting in garbage characters.






https://github.com/user-attachments/assets/ecc90d1b-be31-45c5-94fc-935129f81c32







* **Current Workaround:** A warm boot (Via the microcontroller's rest button) is utilized to reset the HD44780 internal state machine. 

* **Future Action (Tier 2.5/3):** Implement a dedicated raw-nibble software reset sequence inside `LCD_Init()` to bypass the multiplexer and bulletproof the driver against MCU reboots.



---



## Hardware Setup & Pinout



Microcontroller: **STM32 Nucleo-F401RE**



| Peripheral Module | Device Pin | STM32 Pin | Description / Notes |
| :--- | :---: | :---: | :--- |
| **DHT22** | VCC | 3.3V | Main power for the temperature/humidity sensor. |
| **DHT22** | GND | GND | Common ground. |
| **DHT22** | SIG | PA0 | Data wire. Must be bridged with a 10kΩ pull-up resistor to 3.3V. |
| **Freenove LCD** &<br>**MPU-6050 IMU**<br>*(Shared I2C Bus 1)* | VCC | 5.0V / 3.3V | LCD requires 5V rail; MPU-6050 operates on 3.3V rail. |
| **Freenove LCD** &<br>**MPU-6050 IMU** | GND | GND | Common ground. |
| **Freenove LCD** &<br>**MPU-6050 IMU** | SCL | PB8 | I2C Clock Line (Open Drain, Alternate Function 4). |
| **Freenove LCD** &<br>**MPU-6050 IMU** | SDA | PB9 | I2C Data Line (Open Drain, Alternate Function 4). |




---



## Future Roadmap (Current Phase: Inter-Task Telemetry)



### Upcoming (Tier 2)

* **Independent Watchdog (IWDG) Fault Recovery:** While the system currently handles data-wire disconnects via software Safe States, physically severing the ground wire causes a hardware lockup that stalls the RTOS. An Independent Watchdog (IWDG) timer will be reinstated into the architecture to autonomously hard-reset the MCU and recover the system if a pulled ground wire causes the scheduler to freeze.



### Pending Hardware Integration (Suspended)

* **Direct Memory Access (DMA):** An initial attempt was made to migrate I2C transmissions to DMA to reduce CPU blocking. While the I2C hardware handshake was successfully established, a synchronization issue caused the DMA controller to stream corrupted/garbage payload data. This experimental DMA logic was intentionally withheld from the mainline branch to preserve current system stability, but it remains a targeted milestone for future low-power optimization.



---



## Acknowledgements



**AI Attribution:** Google Gemini was utilized as an interactive engineering tutor during this project. Rather than writing the application logic, the AI was strictly prompted to assign tasks, define architectural constraints, explain register-level logic, and review my code. This methodology fostered a deep comprehension of bridging bare-metal hardware protocols with a Real-Time Operating System, specifically focusing on task synchronization, thread-safe queues, and resolving microsecond timing conflicts between the OS scheduler and physical silicon.
