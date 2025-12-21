# Embedded Peripheral & Subsystem TRL Mapping Table (MCXN974)

| Peripheral / Subsystem | TRL-3 (Simulation / Unit Test) | TRL-4 (Hardware Validation) | TRL-5 (System Validation / Stress Test) | TRL-6 (Integrated Prototype Demo) | TRL-7 (System Pre-Flight / Qualification) |
|------------------------|-------------------------------|----------------------------|----------------------------------------|---------------------------------|------------------------------------------|
| **CPU / Cortex-M33 Startup** | Simulate CPU reset in Renode | Flash board, verify bootloader, basic sanity checks | Run full firmware under stress, multi-core context, interrupt-heavy load | Integrated system with peripherals running real tasks | System under operational environment, high-load, endurance tests |
| **SRAM / On-chip Memory** | Simulate reads/writes in Renode | Flash firmware, perform direct read/write tests on MCXN974 | Validate memory under high-load, DMA, cache & peripheral contention | Full system memory usage under integrated workloads | Extended memory stress test, power cycling, fault injection |
| **GPIO / LEDs** | Simulate toggling pins | Flash board, blink on-board LEDs | Drive LEDs under timed sequences or high-frequency toggling | Integrated system status signaling | Qualification of GPIO-driven indicators under environmental stress |
| **UART (flexcomm0-6)** | Simulate TX/RX in Renode | Loopback test on board | Multi-stream communication, high baud rates | UART connected to system peripherals, e.g., sensors, host PC | Stress UART comms in operational scenarios, error handling |
| **I2C / SPI** | Simulate transactions | Connect real sensors on MCXN974 I2C/SPI headers | Stress tests with multiple devices, clock stretching | Full system sensor integration and data logging | Continuous operation with multiple I2C/SPI devices, error recovery tests |
| **CAN / CAN-FD** | Simulate message exchange in Renode | Connect motors/sensors via CAN-FD | Multi-node CAN network, high-speed traffic | System integration of motor/sensor nodes | Pre-flight network verification, error injection, endurance testing |
| **Ethernet / LWIP** | Simulate TCP/IP stack | Bring up Ethernet PHY, ping, DHCP | High throughput, multiple concurrent connections | Integrated Ethernet communication with host and other systems | Qualification under operational network loads, stress TCP/UDP |
| **PWM / Motor Control** | Simulate motor PWM | Control motor via onboard PWM pins | Multi-channel PWM under load, stall detection | Integrated motor control for system demos | Continuous operation tests, thermal and timing qualification |
| **ADC / DAC** | Simulate analog inputs/outputs | Measure signals using on-board ADCs | High-frequency sampling, simultaneous channels | Integrated measurement system, real sensor feedback | Qualification under operational load and noise, calibration verification |
| **Timers / RTC** | Simulate timing events | Verify MCXN974 timers, periodic interrupts | System scheduling under stress | Full system timing coordination | Extended duration, operational timing and synchronization validation |
| **Flash / EEPROM** | Simulate persistent storage | Read/write on-board flash/EEPROM | Wear-leveling, stress cycles | Firmware and configuration updates in full system | Qualification for data retention, power-fail recovery, endurance |
| **Wi-Fi / BLE (if external module attached)** | Simulate network | Connect module and run basic comms | Multi-client stress, throughput testing | System connectivity demo with multiple endpoints | Qualification of network performance in operational conditions |
| **Sensors (IMU, GPS, Cameras via external modules)** | Simulate sensor input | Feed real sensor data | Multi-sensor fusion, high-frequency acquisition | System-level sensor demo, integrated path planning | End-to-end verification, environmental stress testing, fault recovery |
| **Full Firmware / Zephyr App** | Unit tests in Renode | Flash and run basic functions | Stress testing, CI regression, performance benchmarks | Demonstration of full system functionality | System pre-qualification, endurance, integration, and readiness for deployment |
| **Renode Simulation** | Syntax, platform, and subsystem validation | Partial runtime with mocked peripherals | Full runtime simulation with firmware | Optional co-simulation for system demo | Optional regression / pre-flight simulation for operational scenarios |

---

### Notes for TRL-6 / TRL-7:

- **TRL-6:** Usually involves a complete prototype running integrated tasks, e.g., motor control, sensor processing, communication, and UI.
- **TRL-7:** System is tested in realistic conditions, e.g., continuous operation, environmental stress, power cycling, and operational network conditions.
- Some simulations can still be used in TRL-6/7 for edge cases, but **hardware-in-the-loop and end-to-end verification is essential**.
