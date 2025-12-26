<div align="center">

# 🤖 Kobayashi Maru
## Heavy Duty 4WD Robot Platform

[![TRL Level](https://img.shields.io/badge/TRL-3%20Complete-blue?style=for-the-badge)](docs/TRL3_Completion_zephyr_on_MCXN947.md)
[![License](https://img.shields.io/badge/License-GPL%20v3-green?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-MCXN947-orange?style=for-the-badge)](https://www.nxp.com/products/processors-and-microcontrollers/arm-microcontrollers/general-purpose-mcus/mcx-arm-cortex-m/mcx-n-series-microcontrollers/mcx-n94x-and-n54x-mcus-with-dual-core-arm-cortex-m33-edgelock-secure-subsystem-and-neural-processing-unit:MCX-N94X-N54X)
[![RTOS](https://img.shields.io/badge/Framework-Zephyr-blue?style=for-the-badge)](https://www.zephyrproject.org/)
[![Simulation](https://img.shields.io/badge/Simulation-Renode-9cf?style=for-the-badge)](https://renode.io/)

**🎯 Technology Readiness Level:** TRL 3 Complete  
**✅ Status:** TRL-3 completed via simulation — see TRL-3 Completion Assessment  
**🔧 Current State:** Architecture defined • Firmware validated in Renode • CI/CD established  
**🚀 Next Milestone:** TRL 4 hardware validation and timing verification

[Documentation](docs/) • [Architecture](docs/ARCHITECTURE.md) • [TRL-3 Completion](docs/TRL3_Completion_zephyr_on_MCXN947.md)

</div>

---

## 📋 Project Overview

Heavy-duty autonomous 4WD robot platform with modular AI architecture:

- 🧠 **Modular AI processing unit** (Pixel 10 Pro, Raspberry Pi, or Jetson)
- 🛰️ **GPS navigation** and sensor fusion
- 👁️ **Vision processing** with TensorFlow Lite
- 🗺️ **Path planning** (A*/RRT algorithms)
- 🎯 **Pan/tilt turret** for camera/sensor pointing
- 🌐 **Ethernet communication** for platform-independent control
- 🚌 **CAN-FD communication** between motor modules
- 🖥️ **Renode** — Simulation framework for hardware/peripheral modeling and test
- ⚡ **Zephyr RTOS** — Real-time OS and firmware framework for MCU targets

## 🏗️ Hybrid C/C++ Architecture

The firmware uses a recommended hybrid approach common in embedded systems:

| Language | Usage |
|:---------|:------|
| **C++** | MCU modules (Zephyr-based), middleware integration, turret control, high-level motion control, vision + path planning on Pixel 10 Pro |
| **C** | Low-level drivers, ISRs, performance-critical routines |

**Architecture Layers:**


![Hybrid C/C++ Architecture](assets/diagrams/architecture_layers.png)

*Source: [assets/diagrams/architecture_layers.puml](assets/diagrams/architecture_layers.puml)*

<div style="page-break-after: always;"></div>

## 🔧 System Architecture

> **💡 Modular Design:** Ethernet-based communication allows swapping AI processing units (Pixel 10 Pro, Raspberry Pi, Jetson Nano, etc.) without firmware changes. Standard TCP/IP protocol provides platform independence.


**System Block Diagram:**

![System Block Diagram](assets/diagrams/system_block_diagram.png)

*Source: [assets/diagrams/system_block_diagram.puml](assets/diagrams/system_block_diagram.puml)*

For detailed architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## 📁 Project Structure

```
kobayashi_maru/
├── firmware/               # Embedded firmware for FRDM-MCXN947
│   ├── zephyr_app/
│   │   ├── src/                    # Application source
│   │   │   ├── bsp/                # Board Support Package (low-level, C)
│   │   │   ├── drivers/            # Hardware drivers (C)
│   │   │   ├── components/         # Higher-level modules (C or C++)
│   │   │   └── main.c              # Application entry point (C preferred)
│   │   ├── CMakeLists.txt
│   │   └── prj.conf
│   └── config/                     # Configuration files
├── simulation/            # Renode simulation files
│   ├── renode/            # Platform descriptions and scripts (.repl, .resc)
│   └── models/            # Python peripheral models
├── docs/                  # Documentation
│   └── ARCHITECTURE.md    # System architecture details
└── tests/                 # Test files

Note: AI unit applications are developed separately and communicate
via Ethernet TCP/IP (see docs/ARCHITECTURE.md for protocol details)

> Note: The Zephyr application entrypoint is `main.c` (C) by default — prefer C for `main.c`, BSP, and low-level drivers for maximum portability and minimal runtime overhead. Use C++ only for higher-level modules (middleware, control logic). If using C++ enable `CONFIG_CPLUSPLUS=y` and build with a constrained subset (disable exceptions and RTTI: `-fno-exceptions -fno-rtti`).
```

<div style="page-break-after: always;"></div>

## 🔌 Hardware Requirements

### FRDM-MCXN947 Freedom Board

**Processor:** Dual Arm Cortex-M33 @ 150 MHz

**Memory:** 2 MB Flash, 512 KB RAM

**Typical memory map (firmware / Renode):** Flash @ 0x10000000, RAM @ 0x30000000

**Key Features:**
- Ethernet 10/100 (or external PHY module)
- 2x CAN-FD controllers
- Multiple FlexComm (UART, SPI, I2C)
- FlexPWM for servo control

### AI Processing Unit (Modular - Choose One)


#### Option 1: Google Pixel 10 Pro *(Current)*

| Feature | Specification |
|:--------|:--------------|
| **Processor** | Tensor G4 chip with on-device AI acceleration |
| **AI Framework** | TensorFlow Lite / MediaPipe for object detection and tracking |
| **Sensors** | GPS + IMU with 9-axis sensor fusion |
| **Camera** | 50 MP with vision processing |
| **Connectivity** | USB-C to Ethernet adapter |

#### Option 2: Raspberry Pi Compute Module 5 *(Current/Future)*

| Feature | Specification |
|:--------|:--------------|
| **Processor** | ARM Cortex-A78, high-performance general processing |
| **Networking** | Built-in Gigabit Ethernet |
| **Expansion** | GPIO for additional sensors |
| **Software** | Full Linux with ROS support |
| **PCIe** | High-speed peripherals |
| **AI/ML Support** | Improved for vision and robotics workloads |
| **Cost** | ~$60-100 (vs $1000 phone) |

#### Option 3: NVIDIA Jetson Nano / Xavier NX *(Future)*

| Feature | Specification |
|:--------|:--------------|
| **GPU** | 128/384 CUDA cores for acceleration |
| **AI Framework** | TensorRT optimized inference |
| **Networking** | Built-in Gigabit Ethernet |
| **Best For** | Advanced vision AI, multiple cameras |

> **Note:** All options communicate via standard Ethernet TCP/IP. No firmware changes needed to swap platforms.

**Trade-off:**
- For cutting-edge vision and sensor features, the Pixel 10 Pro may outperform the Pi CM5 due to its specialized hardware and software stack.
- For open hardware, flexibility, and community support, the Pi CM5 is a strong choice.

**Recommendation:**
- The architecture is designed to support both platforms, allowing users to select the best fit for their application. This enables flexibility for future upgrades and diverse use cases.

### Motor Modules (x4)
- Brushless DC motors with encoders
- CAN-FD enabled ESC
- Node IDs: 0x100 (FL), 0x101 (FR), 0x102 (RL), 0x103 (RR)

### Pan/Tilt Turret
- High-torque digital servos
- Pan: ±180°, Tilt: -45° to +90°
- CAN-FD Node ID: 0x200

## 🚀 Getting Started

### Prerequisites

- [Renode](https://renode.io/) - For simulation
- Zephyr SDK / `west` toolchain - For firmware compilation (`ZEPHYR_BASE` required)
- AI Unit Application - Android/Python app for chosen platform
- Ethernet network (100 Mbps recommended)

### Running the Simulation

```bash
# Start Renode simulation (from repo root)
cd simulation/renode
renode TRL3_mcxn947_zephyr.resc

# Connect to AI unit terminal (in another terminal)
telnet localhost 3456
```

### Building the Firmware (Zephyr)

```bash
cd firmware/zephyr_app
# Build with west/Zephyr (example target shown)
west build -b frdm_mcxn947/mcxn947/cpu0 .
```

<div style="page-break-after: always;"></div>

## 📡 Communication Protocol

**Ethernet-Based Protocol:** TCP for reliable control commands, UDP for high-frequency sensor data. Platform-independent (works with any device supporting TCP/IP).

#### AI Unit → MCXN947 (Control Messages - TCP)

```cpp
struct ControlMessage {  // 32 bytes total
    uint8_t msg_type;           // 1=GPS, 2=TARGET, 3=CMD, 4=IMU
    uint8_t reserved[3];        // Alignment
    float target_x, target_y;   // Vision: Target position (meters)
    float target_distance;      // Vision: Range to target (meters)
    float heading;              // IMU: Robot orientation (degrees)
    float gps_lat, gps_lon;     // GPS: Current position
    uint8_t command;            // STOP=0, GO=1, FIRE=2, AUTO=3
    uint8_t target_class;       // Object class ID (0-255)
    uint16_t confidence;        // Detection confidence (0-1000)
}; // Sent at 50 Hz = 1.6 KB/s
```

#### MCXN947 → AI Unit (Status Messages - UDP)

```cpp
struct StatusMessage {  // 24 bytes total
    uint8_t msg_type;           // STATUS=1, ACK=2, ERROR=3
    uint8_t robot_state;        // IDLE=0, MANUAL=1, AUTO=2, EMERGENCY=3
    uint16_t battery_mv;        // Battery voltage (millivolts)
    float position_x, position_y;  // Odometry position (meters)
    float velocity;             // Current speed (m/s)
    uint32_t error_flags;       // Error bitfield
    uint32_t timestamp_ms;      // Milliseconds since boot
}; // Sent at 20 Hz = 480 bytes/s
```

**Network Configuration:**

| Device | IP Address | Protocol | Purpose |
|:-------|:-----------|:---------|:--------|
| AI Unit | 192.168.1.100:5000 | TCP Server | Control commands |
| MCXN947 | 192.168.1.10:5001 | UDP | Status broadcasts |

<div style="page-break-after: always;"></div>

## Firmware Architecture

The firmware runs as a Zephyr application on the FRDM-MCXN947 target. Hardware drivers and BSP live under `firmware/zephyr_app/src/` and utilize Zephyr device driver and build model. Emergency stop and high-priority handlers are implemented using Zephyr interrupt API and thread priorities.

### Active Objects (Software Priorities)

| Active Object | Priority | Function |
|:--------------|:--------:|:---------|
| **Supervisor** | 7 | System state machine, safety coordination, heartbeat monitoring |
| **MotorCtrl** | 6 | 4WD motor control via CAN-FD @ 1 kHz (time-critical) |
| **TurretCtrl** | 5 | Pan/tilt servo control, target tracking |
| **EthernetComm** | 4 | TCP/UDP communication with AI unit (platform-agnostic) |
| **SensorFusion** | 3 | Local sensor processing, position estimation |
| **PathPlanner** | 2 | Local obstacle avoidance, waypoint tracking |

**Priority Rationale:** Motor control and turret positioning require real-time guarantees, while network I/O runs at lower priority to prevent jitter in control loops.

**Note:** High-level sensor fusion and AI processing handled on external AI unit.

## 🛠️ Continuous Integration

Automated builds and tests are managed via Jenkins. The pipeline ensures code quality, successful builds, and hardware-in-the-loop simulation before merging changes.

![Jenkins Pipeline Stages](assets/diagrams/Jenkins_pipeline_stages.png)

*Source: Jenkins CI/CD pipeline for Kobayashi Maru project*

## 📄 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

---

<div align="center">

**Built with ❤️ for autonomous robotics**

[⭐ Star this repo](https://github.com/mhouse1/kobayashi_maru) • [🐛 Report Bug](https://github.com/mhouse1/kobayashi_maru/issues) • [💡 Request Feature](https://github.com/mhouse1/kobayashi_maru/issues)

</div>
