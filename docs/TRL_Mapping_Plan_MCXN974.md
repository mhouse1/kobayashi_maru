# Embedded Peripheral & Subsystem TRL Mapping Table (MCXN974)

| Peripheral / Subsystem | TRL-2 (Concept / Paper Study) | TRL-3 (Simulation / Unit Test) | TRL-4 (Hardware Validation / Runtime Checks) | TRL-5 (System Validation / Stress Test) | TRL-6 (Integrated Prototype Demo) | TRL-7 (System Pre-Flight / Qualification) |
|------------------------|-------------------------------|-------------------------------|---------------------------------------------|----------------------------------------|---------------------------------|------------------------------------------|
| **CPU / Cortex-M33 Startup** | Block diagram, reset flow review | Simulate CPU reset in Renode, check boot sequence | Flash board, verify bootloader, basic sanity checks, CPU runtime monitoring | Run full firmware under stress, multi-core context, interrupt-heavy load | Integrated system with peripherals running real tasks | System under operational environment, high-load, endurance tests |
| **SRAM / On-chip Memory** | Memory map and datasheet review | N/A | Flash firmware, perform direct SRAM read/write, verify runtime access | Validate memory under high-load, DMA, cache & peripheral contention | Full system memory usage under integrated workloads | Extended memory stress test, power cycling, fault injection |
| **GPIO / LEDs** | Pinout and function review | N/A | Flash board, toggle LEDs, verify response at runtime | Drive LEDs under timed sequences or high-frequency toggling | Integrated system status signaling | Qualification of GPIO-driven indicators under environmental stress |
| **UART (flexcomm0-6)** | Datasheet and protocol review | N/A | Loopback test on board, verify runtime communication | Multi-stream communication, high baud rates | UART connected to system peripherals, e.g., sensors, host PC | Stress UART comms in operational scenarios, error handling |
| **I2C / SPI** | Protocol review and bus diagrams | N/A | Connect real sensors on MCXN974 I2C/SPI headers, verify functional data transfer | Stress tests with multiple devices, clock stretching | Full system sensor integration and data logging | Continuous operation with multiple I2C/SPI devices, error recovery tests |
| **CAN / CAN-FD** | Network and frame review | N/A | Connect motors/sensors via CAN-FD, verify message delivery | Multi-node CAN network, high-speed traffic | System integration of motor/sensor nodes | Pre-flight network verification, error injection, endurance testing |
| **Ethernet / LWIP** | Protocol and PHY review | N/A | Bring up Ethernet PHY, ping, DHCP, verify basic connectivity | High throughput, multiple concurrent connections | Integrated Ethernet communication with host and other systems | Qualification under operational network loads, stress TCP/UDP |
| **PWM / Motor Control** | Review PWM channels and duty cycles | N/A | Control motor via onboard PWM pins, verify runtime output | Multi-channel PWM under load, stall detection | Integrated motor control for system demos | Continuous operation tests, thermal and timing qualification |
| **ADC / DAC** | Review signal specs and ranges | N/A | Measure signals using on-board ADCs at runtime | High-frequency sampling, simultaneous channels | Integrated measurement system, real sensor feedback | Qualification under operational load and noise, calibration verification |
| **Timers / RTC** | Review timer and RTC specs | N/A | Verify MCXN974 timers, periodic interrupts at runtime | System scheduling under stress | Full system timing coordination | Extended duration, operational timing and synchronization validation |
| **Flash / EEPROM** | Datasheet review | N/A | Read/write on-board flash/EEPROM at runtime | Wear-leveling, stress cycles | Firmware and configuration updates in full system | Qualification for data retention, power-fail recovery, endurance |
| **Wi-Fi / BLE (if external module attached)** | Protocol review | N/A | Connect module, run basic comms, verify responses | Multi-client stress, throughput testing | System connectivity demo with multiple endpoints | Qualification of network performance in operational conditions |
| **Sensors (IMU, GPS, Cameras via external modules)** | Datasheet and dataflow review | N/A | Feed real sensor data, verify correct runtime acquisition | Multi-sensor fusion, high-frequency acquisition | System-level sensor demo, integrated path planning | End-to-end verification, environmental stress testing, fault recovery |
| **Full Firmware / Zephyr App** | Architecture & module review | Unit tests in Renode | Flash and run basic functions, verify boot and key modules | Stress testing, CI regression, performance benchmarks | Demonstration of full system functionality | System pre-qualification, endurance, integration, and readiness for deployment |
| **Renode Simulation** | Syntax & platform concept verification | Syntax, platform, and subsystem validation only | Partial runtime with firmware on real board for validation | Full runtime simulation with firmware | Optional co-simulation for system demo | Optional regression / pre-flight simulation for operational scenarios |

---

### Notes:

- **TRL-3:** **Simulation-only stage**. No real hardware dependencies. Validates functionality of CPU, peripherals, memory, and firmware logic in Renode.  
- **TRL-4:** **First hardware runtime validation**. Uses real MCXN974 board to verify basic boot, firmware loading, and peripheral functionality.  
- **TRL-5 → TRL-7:** Gradually increase system integration, stress, and pre-flight qualification.  

