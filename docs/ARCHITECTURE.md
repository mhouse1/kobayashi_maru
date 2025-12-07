# Heavy-Duty 4WD Robot System Architecture

## Overview

This document describes the system architecture for a heavy-duty 4-wheel drive (4WD) robot with GPS capability, vision processing, path planning, and a pan/tilt turret. The system uses a hybrid architecture combining a Google Pixel 10 Pro smartphone for high-level processing and sensor fusion with an NXP FRDM-MCXN947 Freedom Board for real-time embedded control.

## Hybrid C/C++ Architecture

The firmware uses a hybrid C/C++ approach, which is common in embedded systems:

### C++ is used for:
- **MCU Modules (QP/C++ Framework)** - Active Objects with hierarchical state machines
- **Middleware Integration** - Event-driven communication between subsystems
- **Turret and High-Level Motion Control** - Object-oriented servo control
- **Vision + Path Planning Code** - Complex algorithms on Pixel 10 Pro

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
│                              AI PROCESSING LAYER                                        │
│                              (Google Pixel 10 Pro - Tensor G4)                          │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  ┌──────────────┐            │
│  │   GPS        │  │     IMU      │  │  50MP Camera     │  │  TensorFlow  │            │
│  │   Sensor     │  │  9-axis DOF  │  │  + MediaPipe     │  │    Lite      │            │
│  │   Fusion     │  │    Fusion    │  │  Vision Pipeline │  │  (AI Models) │            │
│  └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘  └──────┬───────┘            │
│         │                 │                   │                   │                     │
│         └─────────────────┴───────────────────┴───────────────────┘                     │
│                                     │                                                   │
│                          ┌──────────▼──────────────────┐                                │
│                          │     Android App             │                                │
│                          │  • Object Detection         │                                │
│                          │  • Target Tracking          │                                │
│                          │  • Sensor Fusion (Kalman)   │                                │
│                          │  • Path Planning (A*/RRT)   │                                │
│                          └──────────┬──────────────────┘                                │
└─────────────────────────────────────┼───────────────────────────────────────────────────┘
                                      │ USB-C 3.2 (1.5 MB/s)
                                      │ Binary Protocol (~1.6 KB/s actual)
                                      │ ControlMessage @ 50 Hz
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
│  │  │ Supervisor │ │ Android    │ │  Sensor    │ │   Path     │ │  Motor     │     │   │
│  │  │    AO      │ │  Comm AO   │ │ Fusion AO  │ │ Planner AO │ │  Ctrl AO   │     │   │
│  │  └─────┬──────┘ └─────┬──────┘ └─────┬──────┘ └─────┬──────┘ └─────┬──────┘     │   │
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

### High-Level Processing Layer (Google Pixel 10 Pro)

| Component | Description |
|-----------|-------------|
| **GPS Module** | Uses Pixel 10 Pro's native GPS for global positioning. Provides latitude, longitude, altitude, speed, and heading data. |
| **Accelerometer/IMU** | Native accelerometer and gyroscope sensors for motion sensing, tilt detection, and orientation tracking. |
| **Vision Processing** | Camera-based object detection, obstacle recognition, and target tracking using Android ML/TensorFlow Lite. |
| **Path Planner** | High-level path planning algorithms (A*, RRT) for navigation between GPS waypoints. |
| **Android App** | Central sensor fusion and command application that aggregates sensor data and communicates with the embedded controller. |

### Real-Time Control Layer (FRDM-MCXN947)

| Component | Description |
|-----------|-------------|
| **Supervisor AO** | Master active object managing system state, emergency stops, and coordination between subsystems. |
| **Android Comm AO** | Handles UART communication with Pixel 10 Pro, parsing commands and sensor data. |
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

### Pixel 10 Pro ↔ FRDM-MCXN947 (UART)

| Direction | Message Type | Format |
|-----------|--------------|--------|
| Pixel → FRDM | GPS Data | `$GPS,lat,lon,alt,speed,heading,sats,fix*` |
| Pixel → FRDM | IMU Data | `$IMU,ax,ay,az,gx,gy,gz,mx,my,mz*` |
| Pixel → FRDM | Vision Target | `$VIS,x,y,w,h,class,conf*` |
| Pixel → FRDM | Command | `$CMD,type,param1,param2,...*` |
| FRDM → Pixel | Status | `$STS,state,battery,error*` |
| FRDM → Pixel | Position | `$POS,x,y,heading,speed*` |

### CAN-FD Message IDs

| Node | TX ID | RX ID | Description |
|------|-------|-------|-------------|
| Master | 0x001 | - | Broadcast commands |
| Motor FL | 0x100 | 0x110 | Speed/position control |
| Motor FR | 0x101 | 0x111 | Speed/position control |
| Motor RL | 0x102 | 0x112 | Speed/position control |
| Motor RR | 0x103 | 0x113 | Speed/position control |
| Turret | 0x200 | 0x210 | Pan/tilt control |

## QP Framework Active Objects

```
┌─────────────────────────────────────────────────────────────────┐
│                    QP Active Object Hierarchy                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Priority 6: ┌──────────────────────────────────────────┐      │
│              │           Supervisor AO                   │      │
│              │  - System state management                │      │
│              │  - Emergency stop handling                │      │
│              │  - Heartbeat monitoring                   │      │
│              └──────────────────────────────────────────┘      │
│                              │                                  │
│  Priority 5: ┌───────────────┴───────────────┐                 │
│              │        Android Comm AO         │                 │
│              │  - UART RX/TX handling         │                 │
│              │  - Protocol parsing            │                 │
│              │  - Command dispatch            │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 4: ┌───────────────┴───────────────┐                 │
│              │       Sensor Fusion AO         │                 │
│              │  - GPS/IMU fusion              │                 │
│              │  - Position estimation         │                 │
│              │  - Obstacle detection          │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 3: ┌───────────────┴───────────────┐                 │
│              │        Path Planner AO         │                 │
│              │  - Waypoint following          │                 │
│              │  - Trajectory generation       │                 │
│              │  - Obstacle avoidance          │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 2: ┌───────────────┴───────────────┐                 │
│              │        Turret Ctrl AO          │                 │
│              │  - Pan/tilt positioning        │                 │
│              │  - Target tracking             │                 │
│              │  - Servo PWM control           │                 │
│              └───────────────────────────────┘                 │
│                              │                                  │
│  Priority 1: ┌───────────────┴───────────────┐                 │
│              │         Motor Ctrl AO          │                 │
│              │  - Speed control               │                 │
│              │  - Position control            │                 │
│              │  - CAN-FD communication        │                 │
│              └───────────────────────────────┘                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

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

### Google Pixel 10 Pro

| Feature | Usage |
|---------|-------|
| GPS | Global positioning |
| Accelerometer | Motion detection, tilt |
| Gyroscope | Angular velocity |
| Camera | Vision processing |
| USB-C | UART connection to FRDM-MCXN947 |
| Processor | High-level computation, ML inference |

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

# Connect to simulated Pixel terminal
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
├── android/
│   ├── app/               # Android application
│   └── interface/         # Communication interface
├── docs/                  # Documentation
└── tests/                 # Test files
```
