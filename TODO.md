```

# Kobayashi Maru - Development Roadmap

## Current Status: TRL 3 (Analytical and Experimental Proof of Concept)

### ✅ Completed (TRL 3)
- Firmware build system (CMake + ARM GNU Toolchain)
- Docker-based development environment
- CI/CD pipeline (Jenkins + Docker)
- Firmware boots successfully in Renode simulation
- Active Object architecture defined (QP framework design)
- Vector table and startup code functional
- Memory layout and linker script working
- Basic BSP structure (stubs only)

### 🎯 Next: TRL 4 (Component Validation in Lab Environment)

**Priority 1: Replace Stubs with Real Implementations**
- [ ] Integrate actual QP/C++ framework (replace qp_stubs.cpp)
- [ ] Implement BSP UART driver for debug output
- [ ] Implement BSP GPIO driver for LEDs and E-stop
- [ ] Implement BSP ADC driver for battery monitoring
- [ ] Implement BSP PWM driver for motor control
- [ ] Implement BSP CAN-FD driver for module communication

**Priority 2: Component Testing**
- [ ] Verify Active Objects execute state machines correctly
- [ ] Test event posting and inter-AO communication
- [ ] Validate time events and periodic heartbeat
- [ ] Test Renode simulation with real QP event loop

**Priority 3: Hardware Validation**
- [ ] Flash firmware to physical FRDM-MCXN947 board
- [ ] Test UART debug console on real hardware
- [ ] Validate GPIO LED indicators
- [ ] Test CAN-FD loopback communication

### 🔮 Future: TRL 5+ (System Integration)
- Android app for telemetry and control
- Motor controller CAN-FD integration
- GPS/IMU sensor fusion
- Autonomous navigation implementation
- Vision-based path planning
```