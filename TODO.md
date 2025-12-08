```

# Kobayashi Maru - Development Roadmap

## Current Status: TRL 2 Complete (Technology Concept Validated via Simulation)

### ✅ TRL 2 COMPLETE - December 7, 2025
**Validation:** Clean build Jenkins #76 - SUCCESS  
**Commit:** `a6051e7` - CLEAN_BUILD parameter for reproducibility  
**Branch:** `trl3_cleanup`

**TRL 2 Achievements:**
- ✅ System architecture defined (ARCHITECTURE.md)
- ✅ Component interfaces specified (Ethernet, CAN-FD protocols)
- ✅ Analytical validation via Renode simulation
- ✅ Proof-of-concept firmware operational
- ✅ Firmware build system (CMake + ARM GNU Toolchain)
- ✅ Docker-based development environment (reproducible)
- ✅ CI/CD pipeline with 9 improvements (Jenkins + Docker)
- ✅ Firmware boots successfully in Renode simulation
- ✅ Active Object architecture defined (QP framework design)
- ✅ Vector table and startup code functional
- ✅ Memory layout and linker script working (464 bytes text, 10256 BSS)
- ✅ Basic BSP structure (stubs for TRL 3-4 implementation)
- ✅ Architecture evolved to Ethernet modular design
- ✅ Documentation complete and consistent
- ✅ Clean build validation passed (no cache, full reproducibility)

**Key Deliverables:**
- 11 commits in `trl3_cleanup` branch
- Jenkins pipeline optimized (caching, metrics, error handling)
- 435 lines of duplicate code removed
- Docker build time optimized (~500MB savings)
- Comprehensive TRL 2 validation checklist created

### 🎯 TRL 3 - Component Validation on Physical Hardware

**Goal:** Validate individual components on real FRDM-MCXN947 hardware  
**Cost:** $24.52 (DigiKey)

**Requirements:**
- Acquire FRDM-MCXN947 development board
- Test CAN-FD communication with motor controllers
- Measure QP/C++ middleware overhead
- Validate peripherals (UART, PWM, ADC, GPIO)
- Prove 50 Hz Ethernet control loop on real silicon

### 🎯 TRL 4 - System Integration in Lab Environment

**Goal:** Full system integration with Zephyr RTOS  
**Architecture Decision:** Switched from QP/C++ to Zephyr RTOS for native Ethernet support

**Priority 1: Zephyr RTOS Integration (Week 1-2)**
- [ ] Install Zephyr SDK 0.16.x and west tool
- [ ] Create Kobayashi Maru Zephyr application structure
- [ ] Configure FRDM-MCXN947 board with DeviceTree
- [ ] Build and flash minimal Zephyr application
- [ ] Verify UART console output and kernel boot

**Priority 2: Ethernet TCP/IP Server (Week 2-3)**
- [ ] Enable networking in prj.conf (lwIP stack)
- [ ] Configure static IP: 192.168.1.10
- [ ] Implement TCP server on port 5001
- [ ] Define ControlMessage (32B) and StatusMessage (24B) structs
- [ ] Parse binary protocol and validate checksums
- [ ] Send UDP status broadcasts @ 20 Hz to AI unit

**Priority 3: Zephyr Driver Configuration (Week 3-4)**
- [ ] UART console via DeviceTree (115200 baud)
- [ ] GPIO: LEDs (heartbeat, error, CAN) and emergency stop button
- [ ] CAN-FD: FlexCAN0 at 500 kbps / 2 Mbps data rate
- [ ] PWM: FlexPWM for motor control signals (20 kHz)
- [ ] ADC: Battery voltage monitoring (0-30V range)
- [ ] All drivers configured via DeviceTree overlay

**Priority 4: Threading Architecture (Week 4-5)**
- [ ] Supervisor thread (priority 7): State machine, heartbeat, coordination
- [ ] Ethernet Comm thread (priority 4): TCP server, message parsing
- [ ] Motor Control thread (priority 6): CAN-FD commands to 4 motors
- [ ] Turret Control thread (priority 5): PWM servo positioning
- [ ] Sensor Fusion thread (priority 3): Placeholder for GPS/IMU
- [ ] Path Planner thread (priority 2): Placeholder for navigation
- [ ] Inter-thread communication via Zephyr message queues

**Priority 5: Renode Simulation Support (Week 6)**
- [ ] Update frdm_mcxn947.repl for Zephyr memory layout
- [ ] Add Ethernet controller to Renode platform
- [ ] Test TCP connectivity from host to simulated board
- [ ] Update Python peripheral models for Zephyr drivers
- [ ] Update Jenkins pipeline for west build commands

**Priority 6: Hardware Validation (Week 7-8)**
- [ ] Acquire FRDM-MCXN947 development board
- [ ] Flash Zephyr firmware via USB (west flash)
- [ ] Verify Ethernet connectivity: ping 192.168.1.10
- [ ] Test TCP server with Python AI unit simulator
- [ ] Validate GPIO LEDs, emergency stop button
- [ ] CAN-FD loopback test (internal loopback mode)
- [ ] Measure performance: CPU, stack, network latency

**Priority 7: Documentation & Sign-off (Week 9)**
- [ ] Update README.md and ARCHITECTURE.md for Zephyr
- [ ] Create Zephyr setup guide (SDK, west, DeviceTree)
- [ ] Document Ethernet protocol (TCP, UDP, message formats)
- [ ] Create TRL 4 validation report with test results
- [ ] Tag release: v2.0-trl4-zephyr

### 🔮 Future: TRL 5+ (System Integration)
- AI unit application for telemetry and control (Android/Python/ROS)
- Motor controller CAN-FD integration
- GPS/IMU sensor fusion
- Autonomous navigation implementation
- Vision-based path planning
```