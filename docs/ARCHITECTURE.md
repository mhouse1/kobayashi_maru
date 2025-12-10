# Heavy-Duty 4WD Robot System Architecture

## Architecture Evolution: Zephyr (TRL2 Baseline) vs FreeRTOS (TRL3 Exploration)

**TRL2 Baseline:**
- The Zephyr-based architecture was used for TRL2, providing built-in networking, device tree, and modular driver support.
- This approach was validated through simulation and is tagged as the TRL2 reference implementation.

**TRL3 Exploration:**
- During TRL3, the team is exploring a FreeRTOS-based architecture for the control MCU, with advanced networking and high-level features offloaded to the AI module (Pixel 10, Raspberry Pi CM5, etc.).
- This exploration is documented in [ADR-001-RTOS-Selection.md](./ADR-001-RTOS-Selection.md) and tracked in the traceability matrix.
- The Zephyr architecture remains available for comparison and potential future use.

**Rationale:**
- TRL2 does not require locking in a single implementation; it requires a validated concept. Zephyr fulfilled this for TRL2.
- TRL3 is the appropriate phase to evaluate alternative implementations (such as FreeRTOS) on hardware.
- All changes and rationale are documented for traceability and review.

**Summary:**
- Zephyr = TRL2 baseline (validated, tagged)
- FreeRTOS = TRL3 exploration (under evaluation)

This dual-architecture approach ensures flexibility and a clear audit trail for future decisions.

## RTOS Selection Decision (Summary)

The decision to use FreeRTOS on the control MCU, with all advanced networking and high-level features offloaded to a modular AI platform (e.g., Pixel 10, Raspberry Pi CM5), is documented in [ADR-001-RTOS-Selection.md](./ADR-001-RTOS-Selection.md).

**Summary:**
- The MCU runs FreeRTOS for real-time, deterministic control (motors, sensors, safety).
- The AI module (running Linux) handles Ethernet, Wi-Fi, BLE, cloud, AI, and user interface tasks.
- This separation improves portability, reliability, and maintainability.
- Zephyr or other feature-rich RTOS options are only needed if future requirements demand more complex MCU features.

See the ADR for full context, options considered, and rationale.

## Overview

This document describes the system architecture for a heavy-duty 4-wheel drive (4WD) robot with GPS capability, vision processing, path planning, and a pan/tilt turret. The system uses a hybrid architecture combining a modular AI processing unit (Google Pixel 10 Pro, Raspberry Pi, or NVIDIA Jetson) for high-level processing and sensor fusion with an NXP FRDM-MCXN947 Freedom Board for real-time embedded control. The AI unit communicates with the MCXN947 via Ethernet TCP/IP, enabling hot-swappable platform changes without firmware modifications.

## Hybrid C/C++ Architecture

The firmware uses a hybrid C/C++ approach, which is common in embedded systems:

### C++ is used for:
- **MCU Modules (QP/C++ Framework)** - Active Objects with hierarchical state machines
- **Middleware Integration** - Event-driven communication between subsystems
- **Turret and High-Level Motion Control** - Object-oriented servo control
- **Vision + Path Planning Code** - Complex algorithms on AI processing unit

### C is used for:
- **Low-level drivers** - Direct hardware register access
- **ISRs (Interrupt Service Routines)** - Time-critical interrupt handlers
- **Performance-critical routines** - Optimized peripheral access
- **Board Support Package (BSP)** - Hardware abstraction layer

```
┌─────────────────────────────────────────────────────────────┐
│                    C++ LAYER (High-Level)                   │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐           │
│  │ QP Active   │ │  Turret     │ │   Path      │           │
│  │  Objects    │ │  Control    │ │  Planner    │           │
│  │  (Classes)  │ │  (Class)    │ │  (Class)    │           │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘           │
│         │               │               │                   │
│  ┌──────┴───────────────┴───────────────┴──────┐           │
│  │            BSP C++ Wrapper Classes          │           │
│  │  (CanFD, Uart, Pwm, Adc, Gpio, Led)        │           │
│  └─────────────────────┬───────────────────────┘           │
└────────────────────────┼────────────────────────────────────┘
                         │ extern "C"
┌────────────────────────┼────────────────────────────────────┐
│                        ▼                                    │
│                    C LAYER (Low-Level)                      │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              bsp_drivers.c / bsp_drivers.h          │   │
│  │  - BSP_canfdInit(), BSP_canfdSend(), ...            │   │
│  │  - BSP_uartInit(), BSP_uartPutchar(), ...           │   │
│  │  - BSP_pwmInit(), BSP_pwmSetDuty(), ...             │   │
│  │  - Interrupt handlers (SysTick_Handler, etc.)       │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│                    HARDWARE REGISTERS                       │
└─────────────────────────────────────────────────────────────┘
```

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                              HEAVY-DUTY 4WD ROBOT SYSTEM                                │
└─────────────────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                           MODULAR AI PROCESSING LAYER                                   │
│          (Google Pixel 10 Pro | Raspberry Pi | NVIDIA Jetson - Hot-Swappable)          │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  ┌──────────────┐            │
│  │   GPS        │  │     IMU      │  │     Camera       │  │  AI Models   │            │
│  │   Sensor     │  │  9-axis DOF  │  │  + MediaPipe     │  │  (TF Lite/   │            │
│  │   Fusion     │  │    Fusion    │  │  Vision Pipeline │  │  TensorRT)   │            │
│  └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘  └──────┬───────┘            │
│         │                 │                   │                   │                     │
│         └─────────────────┴───────────────────┴───────────────────┘                     │
│                                     │                                                   │
│                          ┌──────────▼──────────────────┐                                │
│                          │   AI Unit Application       │                                │
│                          │  • Object Detection         │                                │
│                          │  • Target Tracking          │                                │
│                          │  • Sensor Fusion (Kalman)   │                                │
│                          │  • Path Planning (A*/RRT)   │                                │
│                          └──────────┬──────────────────┘                                │
└─────────────────────────────────────┼───────────────────────────────────────────────────┘
                                      │ Ethernet 100 Mbps TCP/IP
                                      │ 192.168.1.100:5000 ↔ 192.168.1.10:5001
                                      │ ControlMessage @ 50 Hz (TCP)
                                      ▼
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                           REAL-TIME CONTROL LAYER                                       │
│                           (FRDM-MCXN947 Freedom Board)                                  │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                         │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐   │
│  │                        QUANTUM QP FRAMEWORK (Middleware)                        │   │
│  ├─────────────────────────────────────────────────────────────────────────────────┤   │
│  │  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐     │   │
│  │  │ Supervisor │ │ Ethernet   │ │  Sensor    │ │   Path     │ │  Motor     │     │   │
│  │  │    AO      │ │  Comm AO   │ │ Fusion AO  │ │ Planner AO │ │  Ctrl AO   │     │   │
│  │  └─────┬──────┘ └─────┬─────┘ └─────┬──────┘ └─────┬──────┘ └─────┬──────┘     │   │
│  │        │              │              │              │              │             │   │
│  │  ┌─────┴──────────────┴──────────────┴──────────────┴──────────────┴─────┐      │   │
│  │  │                    QP Event-Driven Kernel                              │      │   │
│  │  └────────────────────────────────────────────────────────────────────────┘      │   │
│  └─────────────────────────────────────────────────────────────────────────────────┘   │
│                                                                                         │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐   │
│  │                           BOARD SUPPORT PACKAGE (BSP)                           │   │
│  ├─────────────────────────────────────────────────────────────────────────────────┤   │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐  │   │
│  │  │  CAN-FD  │ │  UART    │ │   PWM    │ │   GPIO   │ │   ADC    │ │  Timer   │  │   │
│  │  │ Driver   │ │ Driver   │ │  Driver  │ │  Driver  │ │  Driver  │ │  Driver  │  │   │
│  │  └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘  │   │
│  └───────┼────────────┼────────────┼────────────┼────────────┼────────────┼────────┘   │
└──────────┼────────────┼────────────┼────────────┼────────────┼────────────┼────────────┘
           │            │            │            │            │            │
           │            │            │            │            │            │
┌──────────┼────────────┼────────────┼────────────┼────────────┼────────────┼────────────┐
│          │            │            │            │            │            │             │
│  ┌───────▼───────┐    │    ┌───────▼───────┐    │    ┌───────▼───────┐    │             │
│  │   CAN-FD      │    │    │    Servo      │    │    │   Battery     │    │             │
│  │   Bus 0       │    │    │   Signals     │    │    │   Monitor     │    │             │
│  └───────┬───────┘    │    └───────┬───────┘    │    └───────────────┘    │             │
│          │            │            │            │                         │             │
│  ┌───────┴──────────────────┐      │            │                         │             │
│  │    CAN-FD MOTOR BUS      │      │            │                         │             │
│  ├──────────────────────────┤      │            │                         │             │
│  │                          │      │            │                         │             │
│  ▼          ▼          ▼    ▼      ▼            ▼                         │             │
│ ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐ ┌──────────────────┐                   │             │
│ │Motor│  │Motor│  │Motor│  │Motor│ │     TURRET       │                   │             │
│ │ FL  │  │ FR  │  │ RL  │  │ RR  │ │  ┌────┐ ┌────┐   │                   │             │
│ │0x100│  │0x101│  │0x102│  │0x103│ │  │Pan │ │Tilt│   │                   │             │
│ └─────┘  └─────┘  └─────┘  └─────┘ │  │Srv │ │Srv │   │                   │             │
│                                     │  └────┘ └────┘   │                   │             │
│  PHYSICAL LAYER                     │     0x200        │                   │             │
│                                     └──────────────────┘                   │             │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

## Component Descriptions

### High-Level Processing Layer (Modular AI Unit)

| Component | Description |
|-----------|-------------|
| **GPS Module** | Native or external GPS for global positioning. Provides latitude, longitude, altitude, speed, and heading data. |
| **Accelerometer/IMU** | Native or external accelerometer and gyroscope sensors for motion sensing, tilt detection, and orientation tracking. |
| **Vision Processing** | Camera-based object detection, obstacle recognition, and target tracking using TensorFlow Lite, TensorRT, or MediaPipe. |
| **Path Planner** | High-level path planning algorithms (A*, RRT) for navigation between GPS waypoints. |
| **AI Unit Application** | Central sensor fusion and command application that aggregates sensor data and communicates with the embedded controller via Ethernet. |

### Real-Time Control Layer (FRDM-MCXN947)

| Component | Description |
|-----------|-------------|
| **Supervisor AO** | Master active object managing system state, emergency stops, and coordination between subsystems. |
| **Ethernet Comm AO** | Handles TCP/IP communication with AI processing unit, parsing commands and sensor data. |
| **Sensor Fusion AO** | Combines GPS, IMU, and vision data for accurate position and orientation estimation. |
| **Path Planner AO** | Real-time path following and trajectory control based on high-level waypoints. |
| **Motor Ctrl AO** | Direct motor speed and position control for all four wheels. |
| **Turret Ctrl AO** | Pan/tilt servo control for turret positioning and tracking. |

### CAN-FD Bus Architecture

```
CAN-FD Bus (5 Mbps)
       │
       ├─── Motor Module FL (0x100) ─── Brushless DC Motor + Encoder
       │
       ├─── Motor Module FR (0x101) ─── Brushless DC Motor + Encoder
       │
       ├─── Motor Module RL (0x102) ─── Brushless DC Motor + Encoder
       │
       ├─── Motor Module RR (0x103) ─── Brushless DC Motor + Encoder
       │
       └─── Turret Module  (0x200) ─── Pan Servo + Tilt Servo
```

## Communication Protocols

### AI Unit ↔ FRDM-MCXN947 (Ethernet TCP/IP)

**Network Configuration:**
- AI Unit: `192.168.1.100:5000` (TCP server)
- MCXN947: `192.168.1.10:5001` (TCP client)
- Protocol: Binary messages with fixed-size structs
- Control Messages: TCP @ 50 Hz (32 bytes)
- Status Broadcasts: UDP @ 20 Hz (24 bytes)

**ControlMessage (AI Unit → MCXN947):**
```c
struct ControlMessage {
    uint8_t header[4];        // "CTRL"
    float motor_speeds[4];    // FL, FR, RL, RR (-1.0 to 1.0)
    float turret_pan;         // Pan angle in degrees
    float turret_tilt;        // Tilt angle in degrees
    uint8_t mode;             // MANUAL, AUTO, EMERGENCY_STOP
    uint8_t checksum;
};
```

**StatusMessage (MCXN947 → AI Unit):**
```c
struct StatusMessage {
    uint8_t header[4];        // "STAT"
    float position_x;
    float position_y;
    float heading;
    uint8_t battery_percent;
    uint8_t system_state;
    uint8_t error_flags;
    uint8_t checksum;
};
```

### CAN-FD Message IDs

| Node | TX ID | RX ID | Description |
|------|-------|-------|-------------|
| Master | 0x001 | - | Broadcast commands |
| Motor FL | 0x100 | 0x110 | Speed/position control |
| Motor FR | 0x101 | 0x111 | Speed/position control |
| Motor RL | 0x102 | 0x112 | Speed/position control |
| Motor RR | 0x103 | 0x113 | Speed/position control |
| Turret | 0x200 | 0x210 | Pan/tilt control |

## Safety and Control Architecture

### Hardware Interrupts (Highest Priority)
**Emergency Stop GPIO Interrupt** - Preempts all software tasks, immediately disables motors
- Response time: 1-2 μs typical (interrupt latency breakdown @ 150 MHz):
  - GPIO edge detection: ~10-20 cycles (67-133 ns)
  - NVIC processing + context save: ~30-50 cycles (200-333 ns)
  - ISR execution (motor disable): ~20-30 cycles (133-200 ns)
  - Total: ~60-100 cycles (0.4-0.67 μs minimum, 1-2 μs typical with pipeline stalls)
  - **Note:** Estimates based on ARM Cortex-M33 documentation; requires hardware measurement for validation
- Directly cuts motor power via hardware disable line
- Posts EMERGENCY_STOP event to Supervisor AO for state cleanup

**Alternative: FPGA-Based Emergency Stop**
- For applications requiring <100 ns response, an FPGA could monitor GPIO and cut motor power through combinatorial logic (no CPU involvement)
- Estimated response: 20-100 ns (synchronizers + gate delays + I/O buffers)
- Trade-off: Adds $10-50 component cost and design complexity
- Not necessary for this application: 1-2 μs response is already 1000x faster than the 1 kHz motor control loop period
- **Note:** FPGA timing estimate; actual performance varies with FPGA architecture and design

### QP Framework Active Objects

```
┌─────────────────────────────────────────────────────────────────┐
│                    QP Active Object Hierarchy                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Priority 7: ┌──────────────────────────────────────────┐      │
│              │           Supervisor AO                   │      │
│              │  - System state management                │      │
│              │  - Emergency stop coordination            │      │
│              │  - Heartbeat monitoring                   │      │
│              │  - Safety interlocks                      │      │
│              └──────────────────────────────────────────┘      │
│                              │                                  │
│  Priority 6: ┌───────────────┴───────────────┐                 │
│              │         Motor Ctrl AO          │                 │
│              │  - Speed control               │                 │
│              │  - Position control            │                 │
│              │  - CAN-FD communication        │                 │
│              │  - Current limiting            │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 5: ┌───────────────┴───────────────┐                 │
│              │        Turret Ctrl AO          │                 │
│              │  - Pan/tilt positioning        │                 │
│              │  - Target tracking             │                 │
│              │  - Servo PWM control           │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 4: ┌───────────────┴───────────────┐                 │
│              │       Ethernet Comm AO         │                 │
│              │  - TCP/IP RX/TX handling       │                 │
│              │  - Binary protocol parsing     │                 │
│              │  - Command dispatch            │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 3: ┌───────────────┴───────────────┐                 │
│              │       Sensor Fusion AO         │                 │
│              │  - GPS/IMU fusion              │                 │
│              │  - Position estimation         │                 │
│              │  - Obstacle detection          │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 2: ┌───────────────┴───────────────┐                 │
│              │        Path Planner AO         │                 │
│              │  - Waypoint following          │                 │
│              │  - Trajectory generation       │                 │
│              │  - Obstacle avoidance          │                 │
│              └───────────────────────────────┘                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

**Priority Rationale:**
- **Supervisor (7):** Highest software priority for state coordination and safety
- **Motor Control (6):** Time-critical motor commands at 1 kHz CAN-FD rate
- **Turret Control (5):** Real-time servo positioning
- **Ethernet (4):** Network I/O lower than control loops to prevent jitter
- **Sensor Fusion (3):** Process sensor data for position estimation
- **Path Planner (2):** Lowest priority - can tolerate delays

## Hardware Specifications

### FRDM-MCXN947 Freedom Board

| Feature | Specification |
|---------|---------------|
| CPU | Dual Arm Cortex-M33 @ 150 MHz |
| Flash | 2 MB |
| RAM | 512 KB |
| CAN-FD | 2x CAN-FD controllers |
| UART | Multiple FlexComm modules |
| PWM | FlexPWM with multiple channels |
| ADC | 16-bit SAR ADC |
| GPIO | Multiple GPIO ports |

### Modular AI Processing Unit Options

**Google Pixel 10 Pro** (Current)
| Feature | Usage |
|---------|-------|
| Tensor G4 | On-device AI acceleration |
| GPS + IMU | Native sensors |
| 50MP Camera | Vision processing |
| Ethernet Adapter | TCP/IP to MCXN947 |

**Raspberry Pi CM5** (Current/Future)
| Feature | Usage |
|---------|-------|
| ARM Cortex-A78 | High-performance general processing |
| Built-in Ethernet | Direct connection |
| GPIO | Sensor expansion |
| Linux | ROS support |
| PCIe | High-speed peripherals |
| Improved AI/ML support | Vision and robotics workloads |

**NVIDIA Jetson** (Future)
| Feature | Usage |
|---------|-------|
| CUDA Cores | GPU acceleration |
| TensorRT | Optimized inference |
| Gigabit Ethernet | High-speed link |
| Multiple cameras | Advanced vision |

## AI Processing Unit Selection: Raspberry Pi CM5 vs Pixel 10 Pro

The robot architecture supports modular AI units, allowing selection based on project needs:

- **Raspberry Pi Compute Module 5 (CM5):**
  - Powerful general-purpose compute, memory, and connectivity
  - Large open-source community and ecosystem
  - Ideal for robotics, control, and custom AI workloads
  - Flexible hardware interfaces (Ethernet, CAN, GPIO, etc.)
  - May require additional effort for advanced camera/sensor integration

- **Google Pixel 10 Pro:**
  - Mature hardware with advanced sensors and camera
  - Billions spent on R&D for vision and AI software
  - Optimized for mobile AI, vision, and sensor fusion
  - Excellent out-of-the-box performance for vision-based tasks
  - Less flexible for custom hardware interfaces, but superior for vision/sensor applications

**Trade-off:**
- For cutting-edge vision and sensor features, the Pixel 10 Pro may outperform the Pi CM5 due to its specialized hardware and software stack.
- For open hardware, flexibility, and community support, the Pi CM5 is a strong choice.

**Recommendation:**
- The architecture is designed to support both platforms, allowing users to select the best fit for their application. This enables flexibility for future upgrades and diverse use cases.

### Motor Modules (x4)

| Feature | Specification |
|---------|---------------|
| Motor Type | Brushless DC |
| Encoder | Magnetic, 4096 PPR |
| Driver | CAN-FD enabled ESC |
| Voltage | 24V-48V |
| Current | Up to 30A continuous |

### Turret

| Feature | Specification |
|---------|---------------|
| Pan Range | ±180° |
| Tilt Range | -45° to +90° |
| Servos | High-torque digital servos |
| Speed | 30°/sec (pan), 20°/sec (tilt) |

## Renode Simulation

The system can be simulated using Renode before deploying to physical hardware:

```bash
# Start simulation
renode simulation/renode/robot_simulation.resc

# Connect to simulated AI unit interface (Ethernet emulation)
telnet localhost 3456
```

### Simulated Components

- **frdm_mcxn947.repl** - Platform description for FRDM-MCXN947
- **motor_model.py** - Simulated motor with encoder feedback
- **turret_model.py** - Simulated pan/tilt servos
- **canfd_model.py** - Simulated CAN-FD bus communication

## Project Structure

```
kobayashi_maru/
├── firmware/
│   ├── src/
│   │   ├── bsp/           # Board Support Package
│   │   ├── drivers/       # Hardware drivers
│   │   ├── subsystems/    # Robot subsystem modules
│   │   └── qp_app/        # QP Active Objects
│   ├── include/           # Header files
│   └── config/            # Configuration files
├── simulation/
│   ├── renode/            # Renode platform files
│   └── models/            # Python simulation models
├── docs/                  # Documentation
└── tests/                 # Test files

Note: AI unit applications (Android/Python/C++) are developed separately
and communicate via standard Ethernet TCP/IP protocol.

> **Architecture Flexibility:**
> The firmware and system architecture support both Zephyr (TRL2 baseline) and FreeRTOS (TRL3 exploration) for the control MCU. The choice of RTOS and middleware is documented and traceable, allowing the project to evaluate and select the best approach as requirements evolve.
```

## Traceability Matrix Reference

This architecture document addresses requirements tracked in [TRACEABILITY_MATRIX.md](TRACEABILITY_MATRIX.md). Key REQ IDs referenced in this document:
- REQ-001: TCP/IP protocol and communication
- REQ-002: CAN-FD bus architecture and drivers
- REQ-003: Emergency stop via GPIO
- REQ-004: Modular AI unit interface
- REQ-005: Turret control protocol and implementation
- REQ-006: Renode simulation platform
- REQ-007: Hardware validation checklist
- REQ-018: RTOS/platform selection and partitioning analysis

Each major section below maps to one or more REQ IDs as noted in headers or inline comments.
