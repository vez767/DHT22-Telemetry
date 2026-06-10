# Low-Level STM32 Environmental Monitor: FreeRTOS Architecture

> **Note:** For the legacy baseline, HIL (Hardware-in-the-Loop) debugging videos, and the foundational 1-Wire timing logic, please refer to [ARCHITECTURE.md](ARCHITECTURE.md).

## Overview
This repository contains a custom Hardware Abstraction Layer (HAL) for the STM32 Nucleo-F401RE, engineered specifically to bridge the gap between the DHT-22's microsecond-level hardware protocols and a Real-Time Operating System (FreeRTOS). 

The system reads raw data from a DHT22 sensor, enforces hardware constraints, and processes the payload safely alongside the FreeRTOS kernel without causing system hangs or data corruption.

## Architecture & Key Features

### 1. The Preemption Shield 
The DHT22 communicates using a clockless, half-duplex single-wire protocol where digital `0`s and `1`s are differentiated purely by the microsecond width of the voltage pulse (28µs vs 70µs). 
* **The RTOS Conflict:** The FreeRTOS SysTick hardware timer fires every 1 millisecond. If the OS preempts the CPU mid-pulse to run the scheduler, the pulse timing is lost, and the 40-bit packet is permanently corrupted.
* **The Engineering Solution:** The data acquisition phase is wrapped in a preemption shield (`taskENTER_CRITICAL()`). This physically disables OS interrupts for the required time to catch the payload before instantly restoring the OS.

### 2. Hybrid Delay Systems 
To maximize CPU availability, this driver utilizes a hybrid timing architecture:
* **Macro-Delays (OS Yielding):** The 1ms+ initialization wake-up pulses utilize `vTaskDelay()`, handing the CPU back to the kernel so parallel system tasks can execute.
* **Micro-Delays (Hardware Polling):** Because FreeRTOS cannot yield in increments smaller than 1ms, a dedicated Basic Timer (`TIM3`) is configured to act as a 1MHz stopwatch. This handles the microscopic 2µs and 80µs handshakes required by the manufacturer's silicon.


### 3. Hardware Thermal Constraints
The DHT22 features an internal thermistor that is highly susceptible to self-heating. If the sensor is polled too rapidly, the internal capacitor heat could potentially corrupt the ambient temperature reading. `vClimateTask` enforces a strict hardware limit by utilizing `vTaskDelay(pdMS_TO_TICKS(2000))` to guarantee a mandatory 2-second thermal cooldown between physical reads.

## Hardware Setup & Pinout

**Microcontroller:** STM32 Nucleo-F401RE

| DHT22 Pin | STM32 Pin | Description / Notes |
| :--- | :--- | :--- |
| **VCC** | 3.3V | Main power for the sensor. |
| **GND** | GND | Common ground. |
| **SIG** | PA0 | Data wire. **Must be bridged with a 10kΩ pull-up resistor to 3.3V.** |

## Future Roadmap (Current Phase: Inter-Task Telemetry)
* **Queue Integration:** Transitioning away from isolated module variables to a centralized FreeRTOS Queue system to safely pass telemetry structs to the display interface without race conditions.
* **I2C Display Integration:** Re-inclusion of the legacy `i2c_lcd.c` driver and wrapping it in a consumer `vDisplayTask` to render the queued telemetry packets.

### Pending Hardware Integration (Suspended)
* **Direct Memory Access (DMA):** An initial attempt was made to migrate I2C transmissions to DMA to reduce CPU blocking. While the I2C hardware handshake was successfully established, a synchronization issue caused the DMA controller to stream corrupted/garbage payload data. This experimental DMA logic was intentionally withheld from the mainline branch to preserve current system stability, but it remains a targeted milestone for future low-power optimization.
