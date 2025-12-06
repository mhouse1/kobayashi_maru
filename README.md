# Kobayashi Maru - Heavy Duty 4WD Robot

Leadership isn't just tested when we win—it's revealed in how we handle the unwinnable. The Kobayashi Maru scenario teaches us that our response to impossible situations often matters more than the outcome itself. True character emerges when doing everything right still isn't enough.

## Project Overview

This project implements a heavy-duty 4-wheel drive (4WD) robot with:

- **GPS navigation** (via Google Pixel 10 Pro)
- **Accelerometer/IMU sensing** (via Google Pixel 10 Pro)
- **Vision processing** (via Google Pixel 10 Pro camera)
- **Path planning** (A*/RRT algorithms)
- **Pan/tilt turret** for camera/sensor pointing
- **CAN-FD communication** between modules
- **Renode simulation** for development and testing
- **Quantum QP Framework** for real-time middleware

## System Architecture

```
┌────────────────────────────────────────────────────────────────────┐
│                    GOOGLE PIXEL 10 PRO                             │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐               │
│  │   GPS   │  │  Accel  │  │  Vision │  │  Path   │               │
│  │         │  │  /IMU   │  │ Process │  │ Planner │               │
│  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘               │
│       └────────────┴────────────┴────────────┘                     │
│                          │ Android App                             │
└──────────────────────────┼─────────────────────────────────────────┘
                           │ UART (115200 baud)
                           ▼
┌────────────────────────────────────────────────────────────────────┐
│                    FRDM-MCXN947 FREEDOM BOARD                      │
│                    (Quantum QP Framework)                          │
│  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌───────────┐          │
│  │Supervisor │ │ Android   │ │  Sensor   │ │   Path    │          │
│  │    AO     │ │  Comm AO  │ │ Fusion AO │ │ Planner   │          │
│  └───────────┘ └───────────┘ └───────────┘ └───────────┘          │
│  ┌───────────┐ ┌───────────┐                                       │
│  │  Motor    │ │  Turret   │                                       │
│  │  Ctrl AO  │ │  Ctrl AO  │                                       │
│  └─────┬─────┘ └─────┬─────┘                                       │
└────────┼─────────────┼─────────────────────────────────────────────┘
         │             │
         │ CAN-FD      │ PWM
         ▼             ▼
┌────────────────┐ ┌────────────────┐
│ MOTOR MODULES  │ │    TURRET      │
│ FL  FR  RL  RR │ │  Pan    Tilt   │
│ 0x100-0x103    │ │     0x200      │
└────────────────┘ └────────────────┘
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
├── android/               # Android app for Pixel 10 Pro
│   ├── app/               # Android application source
│   └── interface/         # Communication interface
├── docs/                  # Documentation
│   └── ARCHITECTURE.md    # System architecture details
└── tests/                 # Test files
```

## Hardware Requirements

### FRDM-MCXN947 Freedom Board
- Dual Arm Cortex-M33 @ 150 MHz
- 2 MB Flash, 512 KB RAM
- 2x CAN-FD controllers
- Multiple FlexComm (UART, SPI, I2C)
- FlexPWM for servo control

### Google Pixel 10 Pro
- GPS for global positioning
- Accelerometer and gyroscope for motion sensing
- Camera for vision processing
- USB-C for UART connection

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
- Android Studio - For Android app development
- QP/C Framework - Real-time embedded framework

### Running the Simulation

```bash
# Start Renode simulation
cd simulation/renode
renode robot_simulation.resc

# Connect to Pixel terminal (in another terminal)
telnet localhost 3456
```

### Building the Firmware

```bash
cd firmware
# Build with ARM GCC (configure toolchain first)
make
```

### Communication Protocol

#### Pixel 10 Pro → FRDM-MCXN947

| Message | Format | Description |
|---------|--------|-------------|
| GPS | `$GPS,lat,lon,alt,speed,heading,sats,fix*` | GPS position data |
| IMU | `$IMU,ax,ay,az,gx,gy,gz,mx,my,mz*` | Accelerometer/gyro/mag data |
| Vision | `$VIS,x,y,w,h,class,conf*` | Detected object info |
| Command | `$CMD,type,param1,param2,...*` | Control commands |

#### FRDM-MCXN947 → Pixel 10 Pro

| Message | Format | Description |
|---------|--------|-------------|
| Status | `$STS,state,battery,error*` | Robot status |
| Position | `$POS,x,y,heading,speed*` | Estimated position |
| Acknowledge | `$ACK,msg*` | Command acknowledgment |

## QP Framework Active Objects

| Active Object | Priority | Function |
|---------------|----------|----------|
| Supervisor | 6 | System state, emergency stops |
| AndroidComm | 5 | UART communication with Pixel |
| SensorFusion | 4 | GPS/IMU data fusion |
| PathPlanner | 3 | Waypoint navigation |
| TurretCtrl | 2 | Pan/tilt servo control |
| MotorCtrl | 1 | 4WD motor control via CAN-FD |

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
