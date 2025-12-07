# TRL 1 Summary - Basic Principles Observed and Reported

**Project:** Kobayashi Maru Heavy-Duty 4WD Robot  
**TRL Level:** Technology Readiness Level 1  
**Date:** December 2025  
**Status:** Completed (Retroactive Documentation)

---

## 1. Technology Selection Rationale

### 1.1 RTOS Selection: Zephyr

**Decision:** Zephyr RTOS (v0.16.x)

**Rationale:**
- **Native Ethernet Support:** Built-in lwIP TCP/IP stack eliminates weeks of integration work
- **MCXN947 Mainline Support:** `boards/arm/frdm_mcxn947` available in Zephyr tree with proven drivers
- **DeviceTree Configuration:** Hardware configuration via declarative DeviceTree files, no manual register setup
- **Rich Driver Ecosystem:** UART, GPIO, CAN-FD, PWM, ADC drivers already implemented and tested
- **BSD Sockets API:** Standard network programming interface reduces development time
- **Production Proven:** Used in commercial robotics and industrial automation
- **Active Community:** Large developer base, extensive documentation, frequent updates

**Alternatives Considered:**
- **QP/C++ Framework:** Excellent for event-driven systems, but requires manual Ethernet stack integration
- **FreeRTOS:** Popular but less comprehensive driver support for MCXN947
- **Bare-metal:** Maximum control but prohibitive development time for network features

**Trade-offs Accepted:**
- Larger code footprint (~150KB vs ~50KB bare-metal) - acceptable with 2MB Flash
- Threading model instead of pure event-driven - simpler for network-heavy applications
- Slightly less deterministic than QP/C++ - mitigated with thread priority configuration

---

### 1.2 Communication Protocol: Ethernet TCP/IP

**Decision:** 100 Mbps Ethernet with TCP/IP (primary) and UDP (status broadcasts)

**Rationale:**
- **Bandwidth:** 12.5 MB/s theoretical throughput far exceeds requirements (~2 KB/s for control)
- **Latency:** <5ms typical for local network, acceptable for 50 Hz control loop (20ms period)
- **Platform Independence:** Standard protocol allows AI unit swapping without firmware changes
- **Reliability:** TCP provides guaranteed delivery for critical control messages
- **Infrastructure:** Standard switches/routers, no custom hardware required
- **Debugging:** Wireshark and standard tools for troubleshooting

**Message Protocol:**
- **ControlMessage (TCP):** 32 bytes @ 50 Hz (1.6 KB/s) - motor commands, turret control
- **StatusMessage (UDP):** 24 bytes @ 20 Hz (480 bytes/s) - robot state, telemetry

**IP Addressing:**
- AI Unit: 192.168.1.100:5000 (client)
- MCXN947: 192.168.1.10:5001 (server)

**Alternatives Considered:**
- **Bluetooth Low Energy:** Insufficient bandwidth (~1 Mbps), higher latency
- **Wi-Fi:** More complex, power-hungry, unnecessary for tethered operation
- **USB:** Not hot-swappable, requires drivers on AI unit
- **CAN-FD:** Used for motor modules, but too low-level for AI unit communication

---

### 1.3 Hardware Platform: FRDM-MCXN947

**Decision:** NXP FRDM-MCXN947 Freedom Board

**Specifications:**
- **MCU:** MCX N947 (Dual Arm Cortex-M33 @ 150 MHz each)
- **Memory:** 2MB Flash, 512KB SRAM
- **Ethernet:** 10/100 Mbps with integrated PHY (ENET QOS)
- **CAN:** 2x CAN-FD controllers (for motor modules)
- **Peripherals:** 10x FlexComm (UART/SPI/I2C), FlexPWM, 16-bit ADC
- **Cost:** ~$100 (readily available)

**Rationale:**
- **Dual-Core Architecture:** Two Cortex-M33 cores enable optimal trade-off between ease of development and hard real-time performance. Core 0 runs Zephyr with Ethernet networking (non-deterministic but feature-rich), while Core 1 can run bare-metal motor control loop with <10 μs jitter. This achieves **better determinism than single-core QP/C++** while retaining Ethernet advantages. TRL 4 uses Core 0 only; Core 1 optimization planned for TRL 5+.
- **Integrated Ethernet PHY:** No external PHY chip required, reduces BOM cost and board complexity
- **CAN-FD Support:** Native dual CAN-FD for motor module communication (1 Mbps+)
- **Sufficient Performance:** 150 MHz per core handles control loops, sensor fusion, TCP/IP stack
- **Memory Capacity:** 2MB Flash accommodates Zephyr (~150KB) with room for application code
- **FlexComm Peripherals:** Flexible serial interfaces for UART debug, future sensor expansion
- **Development Ecosystem:** Official NXP board with SDK support, schematics available

**Alternatives Considered:**
- **STM32H7:** Faster (480 MHz) but more expensive, overkill for requirements
- **ESP32:** Good Wi-Fi but Ethernet requires external PHY, less industrial-grade
- **Raspberry Pi Pico:** Insufficient peripherals (no Ethernet, limited CAN)
- **Teensy 4.1:** Popular but less professional ecosystem than NXP

---

### 1.4 Simulation Platform: Renode

**Decision:** Renode open-source simulation framework

**Rationale:**
- **Deterministic Simulation:** Bit-accurate ARM Cortex-M emulation
- **CI/CD Integration:** Headless operation for Jenkins pipeline testing
- **Hardware Abstraction:** Test firmware before physical hardware arrives
- **Multi-node Networking:** Can simulate Ethernet communication between MCXN947 and AI unit
- **Open Source:** No licensing costs, community-supported
- **Zephyr Integration:** Official support for Zephyr RTOS applications

**Current Status:**
- `.repl` platform file defines MCXN947 peripherals
- `.resc` script automates simulation startup
- Firmware boots successfully in Renode (TRL 3 validated)

---

## 2. Architecture Concept: Modular AI + Embedded Controller

### 2.1 System Architecture

**Design Philosophy:** Separation of concerns with hot-swappable AI processing unit

```
┌────────────────────────────────────────────────────────┐
│         AI PROCESSING UNIT (Modular)                   │
│  • Google Pixel 10 Pro (current)                       │
│  • Raspberry Pi Compute Module (future option)         │
│  • NVIDIA Jetson Nano/Xavier NX (future option)        │
│                                                         │
│  Responsibilities:                                      │
│  • GPS sensor fusion (Kalman filtering)                │
│  • IMU integration (9-axis orientation)                │
│  • Camera vision processing (MediaPipe/TensorFlow)     │
│  • Object detection and tracking                       │
│  • Path planning (A*, RRT algorithms)                  │
│  • High-level decision making                          │
└───────────────────┬────────────────────────────────────┘
                    │ Ethernet 100 Mbps TCP/IP
                    │ ControlMessage @ 50 Hz
                    ▼
┌────────────────────────────────────────────────────────┐
│    FRDM-MCXN947 EMBEDDED CONTROLLER (Real-Time)       │
│    (Zephyr RTOS with Multi-Threading)                 │
│                                                         │
│  Threads (Priority-Based Preemption):                  │
│  • Supervisor Thread (Priority 7) - State machine      │
│  • Motor Control Thread (Priority 6) - CAN-FD output   │
│  • Turret Control Thread (Priority 5) - PWM servos     │
│  • Ethernet Comm Thread (Priority 4) - TCP/IP server   │
│  • Sensor Fusion Thread (Priority 3) - Local IMU       │
│  • Path Planner Thread (Priority 2) - Local fallback   │
│                                                         │
│  Responsibilities:                                      │
│  • Real-time motor control (CAN-FD @ 1 kHz)            │
│  • Safety monitoring (battery, current, faults)        │
│  • Turret servo control (PWM @ 50 Hz)                  │
│  • Network communication handling                      │
│  • Emergency stop and failsafe logic                   │
└───────────────────┬────────────────────────────────────┘
                    │ CAN-FD 1 Mbps
                    ▼
┌────────────────────────────────────────────────────────┐
│           MOTOR MODULES (4x Independent)               │
│  • Front-Left (ID 0x100)                               │
│  • Front-Right (ID 0x101)                              │
│  • Rear-Left (ID 0x102)                                │
│  • Rear-Right (ID 0x103)                               │
│                                                         │
│  Each Module:                                           │
│  • Motor driver (H-bridge or ESC)                      │
│  • Current sensor (inline ADC)                         │
│  • Encoder feedback (speed/position)                   │
└────────────────────────────────────────────────────────┘
```

### 2.2 Key Architectural Principles

**1. Modularity:** AI processing unit is platform-independent, communicates via standard Ethernet
- Swap Pixel 10 Pro → Raspberry Pi → Jetson without firmware changes
- Protocol-defined interface (ControlMessage/StatusMessage structs)
- Hot-swappable during development and deployment

**2. Real-Time Guarantees:** MCXN947 handles time-critical control
- Zephyr thread priorities ensure deterministic response
- Motor control at 1 kHz CAN-FD rate (1ms period)
- Ethernet thread lower priority (4) than motor control (6)

**3. Safety by Design:** Embedded controller enforces limits
- Velocity clamping (max speed limits per motor)
- Current monitoring and overcurrent shutdown
- Watchdog timeout if AI unit disconnects
- Emergency stop via dedicated GPIO input

**4. Scalability:** Architecture supports future expansion
- Additional CAN-FD devices (arm manipulator, sensors)
- Extra FlexComm UARTs for GPS, lidar, ultrasonic
- GPIO expansion for solenoids, lights, relays

---

## 3. Feasibility Assessment

### 3.1 Technical Feasibility

**✅ PROVEN:** All core technologies have been validated in similar applications

**Ethernet Communication:**
- Zephyr lwIP stack used in industrial robots (e.g., Universal Robots, ABB)
- 50 Hz control rate common in robotics (20ms latency budget)
- TCP guarantees delivery, UDP suitable for non-critical status
- **Risk:** LOW - Standard protocol, well-documented

**Zephyr RTOS:**
- Used in production by Tesla (vehicle infotainment), Nordic Semi (IoT), Meta (VR headsets)
- MCXN947 board support in mainline since Zephyr 3.4
- DeviceTree configuration proven on ARM platforms
- **Risk:** LOW - Mature ecosystem, active support

**CAN-FD Motor Control:**
- Automotive standard (ISO 11898-1), proven in harsh environments
- 1 Mbps sufficient for 4 motors @ 1 kHz update rate (32 bytes/motor = 128 KB/s)
- NXP FlexCAN widely used in automotive/industrial
- **Risk:** LOW - Established technology

**AI Processing on Mobile/SBC:**
- MediaPipe runs on Pixel phones (Google official support)
- TensorFlow Lite optimized for mobile ARM processors
- ROS2 runs on Raspberry Pi and Jetson (robotics standard)
- **Risk:** MEDIUM - Performance depends on model complexity

### 3.2 Development Feasibility

**Timeline Estimate (Expert Engineer):**
- TRL 1: Completed (this document)
- TRL 2: 1-2 weeks (subsystem mockups)
- TRL 3: Completed (firmware boots in Renode, December 2025)
- TRL 4: 4-5 weeks (component validation with hardware)
- TRL 5: 6-8 weeks (system integration)
- TRL 6: 8-10 weeks (field testing)
- **Total: 5-6 months to TRL 6** (fully functional prototype)

**Development Tools Available:**
- Zephyr SDK (free, open-source)
- Renode simulator (free, CI/CD ready)
- GCC ARM toolchain (free)
- Jenkins CI/CD (already configured)
- Wireshark (network debugging)
- NXP MCUXpresso (optional, free IDE)

**Hardware Cost:**
- FRDM-MCXN947: $100
- USB-UART adapter: $10
- Ethernet cable: $5
- Oscilloscope (optional): $500-1000
- **Total: <$150 for basic setup**

### 3.3 Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|---------|-----------|
| Ethernet PHY bring-up issues | Medium | High | Use Zephyr sample code, NXP app notes |
| Network latency >20ms | Low | Medium | Use local switch, no internet routing |
| Zephyr learning curve | Low | Medium | Extensive documentation, active forums |
| MCXN947 hardware bugs | Low | High | Use official NXP Freedom Board (tested) |
| Motor CAN-FD communication errors | Medium | High | Implement CRC checks, retry logic |
| AI unit processing delays | High | Medium | Queue commands on MCXN947, use last-known-good |
| Power supply noise | Medium | Medium | Use LDOs, decoupling caps, separate analog/digital grounds |

**Overall Risk Level:** MEDIUM - Standard technologies with proven implementations

---

## 4. Key Assumptions and Constraints

### 4.1 Assumptions

**Network Environment:**
- ✓ Dedicated Ethernet switch (no congestion)
- ✓ Wired connection (no Wi-Fi interference)
- ✓ <2ms Ethernet latency (local LAN)
- ✓ AI unit and MCXN947 on same subnet (192.168.1.0/24)

**AI Processing Unit:**
- ✓ Capable of 50 Hz control loop (20ms max processing time)
- ✓ TCP/IP socket programming support (Python/C++)
- ✓ Sufficient compute for TensorFlow Lite inference (>10 FPS)
- ✓ Ethernet or USB-Ethernet adapter available

**Motor Modules:**
- ✓ CAN-FD capable (ISO 11898-1 compliant)
- ✓ Accept standard motor command messages (velocity, torque)
- ✓ Provide feedback (actual speed, current, faults)
- ✓ Isolated power supplies (motor noise doesn't affect MCU)

**Development Environment:**
- ✓ Linux or macOS for Zephyr SDK (Windows WSL2 acceptable)
- ✓ Hardware debugger (J-Link, OpenOCD, or pyOCD)
- ✓ 5V/3A power supply for FRDM-MCXN947
- ✓ Jenkins server for CI/CD (already configured)

### 4.2 Constraints

**Hardware Constraints:**
- 2MB Flash limit (Zephyr + application must fit)
- 512KB SRAM limit (thread stacks, network buffers, heap)
- 150 MHz CPU speed (deterministic control loop timing required)
- 10/100 Mbps Ethernet only (no Gigabit)
- Limited GPIO pins (FlexComm shared with other functions)

**Software Constraints:**
- Zephyr 0.16.x (or compatible version for MCXN947 support)
- lwIP TCP/IP stack (no alternatives without major rework)
- GCC ARM toolchain (no proprietary compilers)
- Binary protocol (ControlMessage/StatusMessage) - no text/JSON for performance

**Performance Constraints:**
- 50 Hz control rate minimum (20ms max latency AI → motors)
- 1 kHz CAN-FD motor update rate (1ms period)
- 50 Hz turret servo update rate (20ms PWM period)
- 20 Hz status broadcast rate (50ms UDP transmit interval)
- CPU utilization <60% average (headroom for transients)

**Development Constraints:**
- No Android SDK (removed from Docker for space savings)
- Renode simulation only (no hardware-in-the-loop initially)
- Jenkins CI/CD must pass before merging (quality gate)
- Documentation required for TRL advancement (NASA standard)

**Cost Constraints:**
- Development board budget: <$200
- Prefer open-source tools (no license fees)
- Use existing Jenkins infrastructure (no new cloud services)

---

## 5. Expected Outcomes (TRL 1→2→3→4 Progression)

### TRL 1 (Current - Complete)
- ✅ Technology selections documented and justified
- ✅ Architecture concept defined
- ✅ Feasibility assessed (technical, schedule, cost)
- ✅ Risks identified with mitigation strategies
- ✅ Assumptions and constraints documented

### TRL 2 (Target: 1-2 weeks)
- Technology concept formulated
- Subsystem mockups (TCP server stub, motor CAN message simulator)
- Paper studies and simulations
- Proof-of-concept code snippets

### TRL 3 (Complete - December 2025)
- ✅ Firmware boots in Renode simulation
- ✅ Jenkins CI/CD pipeline operational
- ✅ Clean build validated (Build #76 SUCCESS)
- ✅ Repository cleaned and documented

### TRL 4 (Target: 4-5 weeks after TRL 3)
- Component validation in lab environment
- Ethernet TCP/IP server operational on hardware
- Motor CAN-FD communication tested
- DeviceTree drivers configured
- Real-time threading architecture validated
- Hardware/software integration successful

---

## 6. References and Prior Art

**Zephyr RTOS:**
- Official documentation: https://docs.zephyrproject.org/
- MCXN947 board: `boards/arm/frdm_mcxn947/`
- Networking guide: https://docs.zephyrproject.org/latest/connectivity/networking/

**NXP MCXN947:**
- Product page: https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/mcx-arm-cortex-m-mcus/mcx-n-series
- FRDM-MCXN947 User Guide (UM11816)
- MCUXpresso SDK examples

**Robotics Communication:**
- ROS2 uses DDS (similar Ethernet-based architecture)
- ABB robots use Ethernet/IP for controller communication
- Universal Robots UR series uses TCP/IP for external control

**Similar Projects:**
- ROS2 Nav2 stack (path planning + control separation)
- PX4 autopilot (modular architecture, MAVLink protocol)
- ArduPilot (separation of flight controller + companion computer)

---

## 7. Sign-Off

**Author:** [Engineering Team]  
**Date:** December 7, 2025  
**TRL Level:** 1 (Basic Principles Observed and Reported)  
**Next Milestone:** TRL 2 (Technology Concept Formulated)  

**Status:** ✅ COMPLETE

This document establishes the foundation for Kobayashi Maru robot development by documenting technology selections, architecture rationale, and feasibility assessment. All core technologies are proven in production systems with acceptable risk levels. Development can proceed to TRL 2 (concept formulation) and beyond.

---

**Document Revision History:**
- v1.0 (December 7, 2025): Initial creation (retroactive TRL 1 documentation)
