# Compatibility Matrix

This matrix tracks compatibility between major hardware, software, and interface components in the Kobayashi Maru project. Update as new components are added or tested.


## Hardware
| Component Type      | Option/Version         | Supported | Notes                                  |
|--------------------|-----------------------|-----------|----------------------------------------|
| AI Unit            | Pixel 10 Pro          | Planned   | Mature sensors, vision, Android app    |
| AI Unit            | Raspberry Pi CM5      | Planned   | Open hardware, Linux, ROS support      |
| AI Unit            | NVIDIA Jetson Nano    | Planned   | Advanced vision, multiple cameras      |
| Motor Controller   | CAN-FD ESC v1         | Planned   | Node IDs: 0x100-0x103                  |
| Motor Controller   | Generic PWM ESC       | Planned   | For legacy support                     |
| Turret Servo       | Digital PWM (MG995)   | Planned   | Pan/tilt, tested in simulation         |
| Turret Servo       | CAN-FD Servo Module   | Planned   | Future upgrade                         |

## Software
| Component Type      | Option/Version         | Supported | Notes                                  |
|--------------------|-----------------------|-----------|----------------------------------------|
| OS/Firmware        | Zephyr RTOS 0.16.x    | Supported | Mainline, validated in simulation      |
| OS/Firmware        | QP/C++ Framework      | Supported | Used for active objects                |
| Base OS            | Ubuntu 24.04          | Supported | Docker build environment               |
| Python             | 3.12.3                | Supported | Used for simulation, scripting         |
| CMake              | 3.28.3                | Supported | Build system                          |
| GCC                | 13.3.0                | Supported | C/C++ compiler                        |
| Git                | 2.43.0                | Supported | Version control                        |
| Perl               | 5.38.2                | Supported | System scripting                       |
| OpenSSL            | 3.0.13                | Supported | Security libraries                     |
| .NET Runtime       | 8.0.21                | Supported | dotnet-host, dotnet-runtime            |
| Mono               | 6.8.0.105             | Supported | mono-complete, mono-devel              |

## Simulation/Build Tools
| Component Type      | Option/Version         | Supported | Notes                                  |
|--------------------|-----------------------|-----------|----------------------------------------|
| Simulation         | Renode 1.15.0         | Supported | Jenkins pipeline, TRL2 validated       |

## Python Packages
| Component Type      | Option/Version         | Supported | Notes                                  |
|--------------------|-----------------------|-----------|----------------------------------------|
| Python Package     | numpy-2.3.5           | Supported | Simulation, data processing            |
| Python Package     | scipy-1.16.3          | Supported | Simulation, data processing            |
| Python Package     | matplotlib-3.10.7     | Supported | Plotting, visualization                |
| Python Package     | pyserial-3.5          | Supported | Serial communication                   |
| Python Package     | pytest-9.0.2          | Supported | Testing framework                      |

## Protocols
| Component Type      | Option/Version         | Supported | Notes                                  |
|--------------------|-----------------------|-----------|----------------------------------------|
| Protocol           | Ethernet TCP/IP       | Supported | 100 Mbps, validated in simulation      |
| Protocol           | CAN-FD                | Planned   | Architecture defined, driver in progress |
| Protocol           | UART                  | Supported | For debug console                      |
| Protocol           | PWM                   | Supported | For motor and turret control           |
| Protocol           | ADC                   | Supported | For battery monitoring                 |

*Update this matrix as hardware/software is validated or new options are added.*
