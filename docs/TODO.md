```

# Kobayashi Maru - Development Roadmap

### 🎯 TRL 4 - System Integration in Lab Environment

**Goal:** Full system integration with Zephyr RTOS  

**Priority 1: Zephyr RTOS Integration (Week 1-2)**
- [ ] Install Zephyr SDK (match project/CI version; e.g. 0.16.4) and `west` tool
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