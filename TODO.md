```

# Kobayashi Maru - Development Roadmap

## Current Status: TRL 4 (Component Validation in Lab Environment)

### ✅ TRL 3 COMPLETE - December 7, 2025
**Validation:** Clean build Jenkins #76 - SUCCESS  
**Commit:** `a6051e7` - CLEAN_BUILD parameter for reproducibility  
**Branch:** `trl3_cleanup`

**TRL 3 Achievements:**
- ✅ Firmware build system (CMake + ARM GNU Toolchain)
- ✅ Docker-based development environment (reproducible)
- ✅ CI/CD pipeline with 9 improvements (Jenkins + Docker)
- ✅ Firmware boots successfully in Renode simulation
- ✅ Active Object architecture defined (QP framework design)
- ✅ Vector table and startup code functional
- ✅ Memory layout and linker script working (464 bytes text, 10256 BSS)
- ✅ Basic BSP structure (stubs for TRL 4 implementation)
- ✅ Architecture evolved to Ethernet modular design
- ✅ Documentation complete and consistent
- ✅ Clean build validation passed (no cache, full reproducibility)

**Key Deliverables:**
- 11 commits in `trl3_cleanup` branch
- Jenkins pipeline optimized (caching, metrics, error handling)
- 435 lines of duplicate code removed
- Docker build time optimized (~500MB savings)
- Comprehensive TRL 3 validation checklist created

### 🎯 TRL 4 - Component Validation in Lab Environment

**Goal:** Validate individual components and subsystems in laboratory environment

**Priority 1: QP/C++ Framework Integration**
- [ ] Download QP/C++ framework (v7.x) from Quantum Leaps
- [ ] Integrate QP source into firmware/qp/ directory
- [ ] Update CMakeLists.txt to build real QP framework
- [ ] Remove qp_stubs.cpp and implement real QP::run()
- [ ] Verify event loop executes and Active Objects transition states
- [ ] Test QP time events (QTimeEvt) with 1ms SysTick

**Priority 2: BSP Driver Implementation**
- [ ] UART driver: Implement BSP_uartInit(), BSP_uartPutchar() for debug console
- [ ] GPIO driver: Implement BSP_gpioInit(), BSP_ledOn/Off() for status LEDs
- [ ] Timer driver: Implement SysTick for QP time base (1ms tick)
- [ ] ADC driver: Implement BSP_adcInit(), BSP_adcRead() for battery voltage
- [ ] PWM driver: Implement FlexPWM for motor control signals (placeholder)
- [ ] CAN-FD driver: Implement basic BSP_canfdInit() (loopback test)

**Priority 3: Firmware Refactoring (Ethernet Architecture)**
- [ ] Rename android_comm.cpp → ethernet_comm.cpp
- [ ] Rename AndroidCommAO → EthernetCommAO class
- [ ] Update signals: SIG_ANDROID_* → SIG_ETHERNET_*
- [ ] Update Priority enum: ANDROID_COMM → ETHERNET_COMM
- [ ] Update all references in supervisor.cpp and other Active Objects
- [ ] Update comments to reflect Ethernet TCP/IP communication

**Priority 4: Component Testing in Simulation**
- [ ] Verify Supervisor AO transitions through IDLE → READY → RUNNING states
- [ ] Test event posting between Active Objects (Supervisor → MotorCtrl)
- [ ] Validate 1s heartbeat in Supervisor using QTimeEvt
- [ ] Test Renode UART output shows "Supervisor: IDLE" → "Supervisor: RUNNING"
- [ ] Verify QP event loop runs (no longer infinite WFI loop)
- [ ] Test emergency stop signal propagation

**Priority 5: Hardware Validation (Physical FRDM-MCXN947)**
- [ ] Acquire FRDM-MCXN947 development board
- [ ] Flash firmware using OpenOCD or MCUXpresso
- [ ] Connect UART console (115200 baud) and verify debug output
- [ ] Validate LED blinks (heartbeat indicator)
- [ ] Test GPIO emergency stop button (SW2)
- [ ] Validate CAN-FD loopback (TX → RX on same controller)
- [ ] Measure timing: verify 1ms SysTick accuracy with oscilloscope

### 🔮 Future: TRL 5+ (System Integration)
- AI unit application for telemetry and control (Android/Python/ROS)
- Motor controller CAN-FD integration
- GPS/IMU sensor fusion
- Autonomous navigation implementation
- Vision-based path planning
```