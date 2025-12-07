# Kobayashi Maru - Heavy Duty 4WD Robot

Leadership isn't just tested when we win—it's revealed in how we handle the unwinnable. The Kobayashi Maru scenario teaches us that our response to impossible situations often matters more than the outcome itself. True character emerges when doing everything right still isn't enough.

## Project Overview

This project implements a heavy-duty 4-wheel drive (4WD) robot with:

- **Modular AI processing unit** (Pixel 10 Pro, Raspberry Pi, or Jetson)
- **GPS navigation** and sensor fusion
- **Vision processing** with TensorFlow Lite
- **Path planning** (A*/RRT algorithms)
- **Pan/tilt turret** for camera/sensor pointing
- **Ethernet communication** for platform-independent control
- **CAN-FD communication** between motor modules
- **Renode simulation** for development and testing
- **Quantum QP/C++ Framework** for real-time middleware

## Hybrid C/C++ Architecture

The firmware uses a recommended hybrid approach common in embedded systems:

| Language | Usage |
|----------|-------|
| **C++** | MCU modules (QP framework), middleware integration, turret control, high-level motion control, vision + path planning on Pixel 10 Pro |
| **C** | Low-level drivers, ISRs, performance-critical routines |

```
┌──────────────────────────────────────────────────────┐
│           C++ LAYER (QP/C++ Active Objects)          │
│  Robot::MotorCtrlAO, TurretCtrlAO, PathPlannerAO... │
│  BSP::CanFD, BSP::Uart, BSP::Pwm (C++ wrappers)     │
├──────────────────────────────────────────────────────┤
│              C LAYER (Low-Level Drivers)             │
│  bsp_drivers.c - Direct hardware access              │
│  ISRs: SysTick_Handler, CANFD0_IRQHandler, etc.      │
└──────────────────────────────────────────────────────┘
```

## System Architecture

**Modular AI Architecture:** Ethernet-based communication allows swapping AI processing units (Pixel 10 Pro, Raspberry Pi, Jetson Nano, etc.) without firmware changes. Standard TCP/IP protocol provides platform independence.

```
┌──────────────────────────────────────────────────────────────────┐
│           AI PROCESSING UNIT (Modular - Hot-swappable)          │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐    │
│  │ Google Pixel   │  │ Raspberry Pi   │  │ Jetson Nano    │    │
│  │ 10 Pro         │  │ Compute Module │  │ / Xavier NX    │    │
│  │ (Current)      │  │ (Future)       │  │ (Future)       │    │
│  └────────────────┘  └────────────────┘  └────────────────┘    │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │   GPS    │    IMU    │   Camera   │  TensorFlow Lite   │    │
│  │  Fusion  │   Fusion  │  + Vision  │   / MediaPipe      │    │
│  └─────┬────────────┬──────────┬────────────────┬──────────┘    │
│        └────────────┴──────────┴────────────────┘                │
│                          │ AI App                                │
│                    ┌─────▼────────────┐                          │
│                    │ Sensor Fusion    │                          │
│                    │ Object Detection │                          │
│                    │ Path Planning    │                          │
│                    └──────────────────┘                          │
└──────────────────────────┼───────────────────────────────────────┘
                           │ Ethernet (100 Mbps - 12.5 MB/s)
                           │ TCP/IP or UDP
                           │ ControlMessage @ 50 Hz (~1.6 KB/s)
                           ▼
┌──────────────────────────────────────────────────────────────────┐
│              FRDM-MCXN947 FREEDOM BOARD (Motor Brain)            │
│                    (Quantum QP/C++ Framework)                    │
│  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐       │
│  │Supervisor │ │ Ethernet  │ │   Path    │ │  Motor    │       │
│  │    AO     │ │  Comm AO  │ │ Planner   │ │  Ctrl AO  │       │
│  │  (State)  │ │ (TCP/UDP) │ │ (Local)   │ │ (CAN-FD)  │       │
│  └───────────┘ └───────────┘ └───────────┘ └─────┬─────┘       │
│  ┌───────────┐                                    │             │
│  │  Turret   │                                    │             │
│  │  Ctrl AO  │                                    │             │
│  └─────┬─────┘                                    │             │
└────────┼──────────────────────────────────────────┼─────────────┘
         │ PWM                                      │ CAN-FD
         ▼                                          ▼
┌────────────────┐                          ┌────────────────┐
│    TURRET      │                          │ MOTOR MODULES  │
│  Pan    Tilt   │                          │ FL  FR  RL  RR │
│     0x200      │                          │ 0x100-0x103    │
└────────────────┘                          └────────────────┘
```

For detailed architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Project Structure

```
kobayashi_maru/
├── firmware/               # Embedded firmware for FRDM-MCXN947
│   ├── src/
│   │   ├── bsp/           # Board Support Package
│   │   ├── drivers/       # Hardware drivers
│   │   ├── subsystems/    # Robot subsystem modules
│   │   ├── qp_app/        # QP Active Objects
│   │   └── main.c         # Application entry point
│   ├── include/           # Header files
│   └── config/            # Configuration files
├── simulation/            # Renode simulation files
│   ├── renode/            # Platform descriptions and scripts
│   └── models/            # Python peripheral models
├── docs/                  # Documentation
│   └── ARCHITECTURE.md    # System architecture details
└── tests/                 # Test files

Note: AI unit applications are developed separately and communicate
via Ethernet TCP/IP (see docs/ARCHITECTURE.md for protocol details)
```

## Hardware Requirements

### FRDM-MCXN947 Freedom Board
- Dual Arm Cortex-M33 @ 150 MHz
- 2 MB Flash, 512 KB RAM
- **Ethernet 10/100** (or external PHY module)
- 2x CAN-FD controllers
- Multiple FlexComm (UART, SPI, I2C)
- FlexPWM for servo control

### AI Processing Unit (Modular - Choose One)

**Option 1: Google Pixel 10 Pro** (Current)
- **Tensor G4 chip** - On-device AI acceleration
- **TensorFlow Lite / MediaPipe** - Object detection, tracking
- **GPS + IMU** - 9-axis sensor fusion
- **50 MP camera** - Vision processing
- **USB-C to Ethernet adapter** - Network connectivity

**Option 2: Raspberry Pi Compute Module 4** (Future)
- **Quad-core ARM Cortex-A72** @ 1.5 GHz
- **Built-in Ethernet** (Gigabit on CM4)
- **GPIO expansion** for additional sensors
- **Full Linux** - ROS support, easier development
- **Lower cost** - ~$35-75 vs $1000 phone

**Option 3: NVIDIA Jetson Nano / Xavier NX** (Future)
- **GPU acceleration** - 128/384 CUDA cores
- **TensorRT** - Optimized inference
- **Gigabit Ethernet** built-in
- **Best for** - Advanced vision AI, multiple cameras

**All options communicate via standard Ethernet TCP/IP** - No firmware changes needed to swap

### Motor Modules (x4)
- Brushless DC motors with encoders
- CAN-FD enabled ESC
- Node IDs: 0x100 (FL), 0x101 (FR), 0x102 (RL), 0x103 (RR)

### Pan/Tilt Turret
- High-torque digital servos
- Pan: ±180°, Tilt: -45° to +90°
- CAN-FD Node ID: 0x200

## Getting Started

### Prerequisites

- [Renode](https://renode.io/) - For simulation
- ARM GCC Toolchain - For firmware compilation
- AI Unit Application - Android/Python app for chosen platform
- QP/C Framework - Real-time embedded framework
- Ethernet network (100 Mbps recommended)

### Running the Simulation

```bash
# Start Renode simulation
cd simulation/renode
renode robot_simulation.resc

# Connect to AI unit terminal (in another terminal)
telnet localhost 3456
```

### Building the Firmware

```bash
cd firmware
# Build with ARM GCC (configure toolchain first)
make
```

### Communication Protocol

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
- AI Unit: 192.168.1.100:5000 (TCP Server for commands)
- MCXN947: 192.168.1.10:5001 (UDP for status broadcasts)

## QP Framework Active Objects

| Active Object | Priority | Function |
|---------------|----------|----------|
| Supervisor | 6 | System state machine (IDLE/MANUAL/AUTO/EMERGENCY) |
| EthernetComm | 5 | TCP/UDP communication with AI unit (platform-agnostic) |
| PathPlanner | 3 | Local obstacle avoidance, waypoint tracking |
| TurretCtrl | 2 | Pan/tilt servo control (aim at targets) |
| MotorCtrl | 1 | 4WD motor control via CAN-FD buses |

**Note:** Sensor fusion and AI processing handled on external AI unit.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
