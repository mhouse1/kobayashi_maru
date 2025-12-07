# TRL 4 Implementation Plan
**Technology Readiness Level 4:** Component and/or breadboard validation in laboratory environment

**Start Date:** December 7, 2025  
**Target Completion:** Q1 2026  
**Previous:** TRL 3 completed (Build #76, commit `a6051e7`)  
**Architecture Decision:** Switched to Zephyr RTOS for native Ethernet support

---

## TRL 4 Definition (NASA Standard)

**Goal:** Validate individual technology components and/or basic subsystems in a laboratory environment with representative conditions.

**Exit Criteria:**
- Stand-alone component validation complete
- Integration with realistic supporting elements
- Readiness for system-level integration
- Performance measured against requirements

---

## Architecture Change: Zephyr RTOS

**Rationale for switching from QP/C++ to Zephyr:**
- ✅ Native Ethernet stack (lwIP built-in) - working in days vs weeks
- ✅ MCXN947 support in mainline (boards/arm/frdm_mcxn947)
- ✅ TCP/IP sockets API - simpler network programming
- ✅ DeviceTree hardware configuration - no code changes for pins
- ✅ Rich driver ecosystem - UART, GPIO, CAN, PWM, ADC already implemented
- ✅ Better for network-heavy robotics applications
- ✅ Larger community and production-proven

**Trade-offs accepted:**
- ⚠️ Larger footprint (~150KB vs ~50KB with QP) - acceptable for 2MB Flash
- ⚠️ Threading model instead of event-driven - simpler for network I/O
- ⚠️ Less deterministic than QP **on single-core** - mitigated with thread priorities for TRL 4

**Dual-Core Determinism Advantage:**
- Single-core Zephyr (TRL 4): Network stack shares CPU with motor control (~50-200 μs jitter)
- **Dual-core configuration (TRL 5+):** Core 1 runs bare-metal motor control with **<10 μs jitter**
  - Core 0: Zephyr + Ethernet (non-deterministic networking isolated)
  - Core 1: Bare-metal 1 kHz control loop (zero network interruptions)
  - Inter-core communication via mailbox (predictable latency)
  - **Result: Better determinism than single-core QP/C++ while keeping Ethernet benefits**
- TRL 4 uses **Core 0 only** to validate architecture before dual-core optimization
- Core 1 remains in reset/idle state during TRL 4
- Asymmetric multiprocessing (AMP) with Zephyr + OpenAMP or Zephyr + bare-metal

---

## Phase 1: Zephyr RTOS Integration (Week 1-2)

### Objectives
Set up Zephyr development environment and get basic firmware running with Ethernet support.

### Tasks
1. **Install Zephyr SDK**
   - [ ] Install Zephyr SDK 0.16.x on development machine
   - [ ] Install west tool: `pip3 install west`
   - [ ] Initialize Zephyr workspace: `west init ~/zephyrproject`
   - [ ] Update Zephyr: `west update`
   - [ ] Install dependencies: `pip3 install -r ~/zephyrproject/zephyr/scripts/requirements.txt`

2. **Create Kobayashi Maru Application**
   - [ ] Create application directory: `kobayashi_maru_zephyr/`
   - [ ] Create `prj.conf` with CONFIG_NETWORKING=y, CONFIG_NET_TCP=y
   - [ ] Create minimal `main.c` with thread setup
   - [ ] Create `CMakeLists.txt` for Zephyr build system
   - [ ] Create `.west/config` pointing to zephyr base

3. **FRDM-MCXN947 Board Configuration**
   - [ ] Use existing board: `boards/arm/frdm_mcxn947` from Zephyr tree
   - [ ] Review DeviceTree: `frdm_mcxn947.dts` (pins, Ethernet, CAN)
   - [ ] Create overlay if needed: `frdm_mcxn947.overlay` for custom pins
   - [ ] Verify Ethernet PHY configuration (ENET QOS controller)

4. **Build System Integration**
   - [ ] Update Docker image to include Zephyr SDK
   - [ ] Add west to PATH in Dockerfile
   - [ ] Update Jenkins pipeline for Zephyr builds: `west build -b frdm_mcxn947`
   - [ ] Configure CMake for Zephyr toolchain

5. **First Boot Test**
   - [ ] Build minimal application: `west build -b frdm_mcxn947`
   - [ ] Flash to board: `west flash`
   - [ ] Verify UART console output
   - [ ] Confirm Zephyr kernel boots

**Expected Outcome:** Zephyr firmware boots on FRDM-MCXN947, UART console operational

---

## Phase 2: Ethernet TCP/IP Server (Week 2-3)

### Objectives
Implement TCP server on MCXN947 to communicate with AI processing unit.

### Tasks
1. **Network Configuration**
   - [ ] Enable networking in `prj.conf`:
     ```
     CONFIG_NETWORKING=y
     CONFIG_NET_IPV4=y
     CONFIG_NET_TCP=y
     CONFIG_NET_SOCKETS=y
     CONFIG_NET_LOG=y
     ```
   - [ ] Configure static IP: 192.168.1.10 in `prj.conf`
   - [ ] Set netmask: 255.255.255.0, no gateway needed

2. **TCP Server Implementation**
   - [ ] Create `ethernet_comm.c` with TCP server thread
   - [ ] Implement socket creation: `socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)`
   - [ ] Bind to port 5001: `bind()` and `listen()`
   - [ ] Accept connections: `accept()` blocking call
   - [ ] Implement receive loop: `recv()` for ControlMessage (32 bytes)

3. **Message Protocol**
   - [ ] Define `struct ControlMessage` (32 bytes):
     - Header: "CTRL" (4 bytes)
     - Motor speeds: float[4] (16 bytes)
     - Turret pan/tilt: float[2] (8 bytes)
     - Mode: uint8_t (1 byte)
     - Checksum: uint8_t (1 byte)
     - Padding: 2 bytes
   - [ ] Implement checksum validation
   - [ ] Parse binary message and extract commands

4. **Status Broadcast (UDP)**
   - [ ] Create UDP socket for status broadcasts
   - [ ] Implement `struct StatusMessage` (24 bytes)
   - [ ] Send status every 50ms to 192.168.1.100:5000
   - [ ] Include: position, heading, battery, state, errors

5. **Testing**
   - [ ] Use `nc` (netcat) to connect from PC: `nc 192.168.1.10 5001`
   - [ ] Send test ControlMessage bytes
   - [ ] Verify parsing and logging
   - [ ] Use Wireshark to capture packets

**Expected Outcome:** TCP server accepts connections, parses ControlMessage, sends StatusMessage

---

## Phase 3: Zephyr Driver Configuration (Week 3-4)

### Objectives
Configure and test Zephyr drivers for all robot peripherals using DeviceTree.

### UART Console (DeviceTree)
**Purpose:** Debug output via FlexComm UART

**Implementation:**
- [ ] Verify UART node in `frdm_mcxn947.dts`:
  ```dts
  &lpuart1 {
      status = "okay";
      current-speed = <115200>;
  };
  ```
- [ ] Enable console in `prj.conf`: `CONFIG_SERIAL=y`, `CONFIG_UART_CONSOLE=y`
- [ ] Use `printk()` for logging: `printk("Kobayashi Maru v2.0 - TRL 4\n");`

**Testing:**
- [ ] Connect UART pins to USB-UART adapter
- [ ] Open serial terminal (115200 8N1)
- [ ] Verify boot messages and log output

### GPIO (LEDs and Buttons)
**Purpose:** Status indicators and emergency stop

**Implementation:**
- [ ] Define GPIO in DeviceTree overlay:
  ```dts
  / {
      leds {
          compatible = "gpio-leds";
          led0: led_0 { gpios = <&gpio0 10 GPIO_ACTIVE_HIGH>; };
          led1: led_1 { gpios = <&gpio0 11 GPIO_ACTIVE_HIGH>; };
      };
      buttons {
          compatible = "gpio-keys";
          estop: button_0 { gpios = <&gpio1 3 GPIO_ACTIVE_LOW>; };
      };
  };
  ```
- [ ] Use Zephyr GPIO API:
  ```c
  const struct device *led = DEVICE_DT_GET(DT_ALIAS(led0));
  gpio_pin_configure_dt(led, GPIO_OUTPUT_ACTIVE);
  gpio_pin_toggle_dt(led);
  ```

**Testing:**
- [ ] Blink LED0 at 1 Hz (heartbeat)
- [ ] LED1 indicates Ethernet connection
- [ ] Button press triggers emergency stop

### CAN-FD Driver
**Purpose:** Motor module communication

**Implementation:**
- [ ] Enable CAN in `prj.conf`:
  ```
  CONFIG_CAN=y
  CONFIG_CAN_FD_MODE=y
  ```
- [ ] Configure CAN node in DeviceTree:
  ```dts
  &flexcan0 {
      status = "okay";
      bus-speed = <500000>;
      bus-speed-data = <2000000>;
  };
  ```
- [ ] Use Zephyr CAN API:
  ```c
  const struct device *can = DEVICE_DT_GET(DT_NODELABEL(flexcan0));
  struct can_frame frame = {.id = 0x100, .dlc = 8};
  can_send(can, &frame, K_MSEC(100), NULL, NULL);
  ```

**Testing:**
- [ ] Loopback test (internal loopback mode)
- [ ] Send test frame, verify reception
- [ ] Measure CAN timing with oscilloscope

### PWM (Motor Control Placeholder)
**Purpose:** Future motor ESC control

**Implementation:**
- [ ] Enable PWM in `prj.conf`: `CONFIG_PWM=y`
- [ ] Configure PWM in DeviceTree:
  ```dts
  &flexpwm0 {
      status = "okay";
      pinctrl-0 = <&flexpwm0_default>;
  };
  ```
- [ ] Use Zephyr PWM API:
  ```c
  const struct device *pwm = DEVICE_DT_GET(DT_NODELABEL(flexpwm0));
  pwm_set_cycles(pwm, channel, period, pulse, 0);
  ```

**Testing:**
- [ ] Set 50% duty cycle, verify with scope (20 kHz)
- [ ] Sweep 0-100%, measure linearity

### ADC (Battery Monitor)
**Purpose:** Battery voltage reading

**Implementation:**
- [ ] Enable ADC in `prj.conf`: `CONFIG_ADC=y`
- [ ] Configure ADC channel in DeviceTree
- [ ] Read voltage with `adc_read()` API
- [ ] Scale to 0-30V range (voltage divider)

**Testing:**
- [ ] Apply known voltage, verify reading (±2% accuracy)

**Expected Outcome:** All drivers configured via DeviceTree, tested and operational

---

## Phase 4: Threading Architecture (Week 4-5)

### Objectives
Implement multi-threaded architecture for robot control subsystems.

### Thread Design
```c
// Thread priorities (higher number = higher priority)
#define PRIORITY_SUPERVISOR     7  // Highest - system coordination
#define PRIORITY_MOTOR_CTRL     6  // Real-time motor control
#define PRIORITY_TURRET_CTRL    5  // Turret positioning
#define PRIORITY_ETHERNET_COMM  4  // Network communication
#define PRIORITY_SENSOR_FUSION  3  // GPS/IMU processing
#define PRIORITY_PATH_PLANNER   2  // Path planning

// Stack sizes
#define STACK_SIZE_SUPERVISOR   1024
#define STACK_SIZE_MOTOR_CTRL   1024
#define STACK_SIZE_ETHERNET     2048  // Larger for network buffers
#define STACK_SIZE_SENSOR       1536
#define STACK_SIZE_PATH         2048
#define STACK_SIZE_TURRET       1024
```

### Supervisor Thread
**Purpose:** System state machine and coordination

**States:**
- INIT → Initializing subsystems
- IDLE → Ready, waiting for start command
- RUNNING → Normal operation
- ERROR → Fault detected, safe mode

**Implementation:**
- [ ] Create thread: `K_THREAD_DEFINE(supervisor_thread, ...)`
- [ ] Implement state machine with `switch` statement
- [ ] Use message queues for inter-thread communication:
  ```c
  K_MSGQ_DEFINE(supervisor_msgq, sizeof(msg_t), 10, 4);
  ```
- [ ] Periodic 1s heartbeat using `k_timer`
- [ ] Monitor all subsystems for faults

**Testing:**
- [ ] Verify state transitions: INIT → IDLE → RUNNING
- [ ] Trigger emergency stop, confirm ERROR state
- [ ] Heartbeat blinks LED every 1s

### Ethernet Comm Thread
**Purpose:** TCP server for AI unit communication

**Implementation:**
- [ ] Accept TCP connections from AI unit (192.168.1.100)
- [ ] Receive ControlMessage (32 bytes) blocking call
- [ ] Parse commands and post to message queues:
  ```c
  k_msgq_put(&motor_msgq, &motor_cmd, K_NO_WAIT);
  ```
- [ ] Send StatusMessage via UDP every 50ms
- [ ] Handle disconnections and reconnections

**Testing:**
- [ ] Connect from PC using `nc 192.168.1.10 5001`
- [ ] Send binary ControlMessage, verify parsing
- [ ] Monitor UDP broadcasts with Wireshark

### Motor Control Thread
**Purpose:** CAN-FD motor module communication

**Implementation:**
- [ ] Receive motor commands from message queue
- [ ] Send CAN frames to 4 motor controllers (0x100-0x103):
  ```c
  struct can_frame frame = {
      .id = MOTOR_FL_ID,
      .data = {speed_cmd, ...}
  };
  can_send(can_dev, &frame, K_MSEC(10), NULL, NULL);
  ```
- [ ] Receive status from motors (encoder feedback)
- [ ] Implement timeout detection (100ms)

**Testing:**
- [ ] Loopback: send command, verify CAN frame
- [ ] Measure command latency (< 10ms)

### Turret Control Thread
**Purpose:** Pan/tilt servo positioning

**Implementation:**
- [ ] Receive target angles from Supervisor
- [ ] Convert angles to PWM duty cycles
- [ ] Smooth transitions (slew rate limiting)
- [ ] Read position feedback (if available)

### Path Planner Thread (Placeholder)
**Purpose:** Future autonomous navigation

**Implementation:**
- [ ] Placeholder: log received waypoints
- [ ] To be implemented in TRL 5

### Sensor Fusion Thread (Placeholder)
**Purpose:** Future GPS/IMU integration

**Implementation:**
- [ ] Placeholder: simulate sensor data
- [ ] To be implemented in TRL 5

**Expected Outcome:** All threads execute, communicate via message queues, coordinated by Supervisor

---

## Phase 5: Renode Simulation Support (Week 6)

### Objectives
Update Renode simulation to support Zephyr firmware with Ethernet.

### Tasks
1. **Zephyr Boot in Renode**
   - [ ] Update `frdm_mcxn947.repl` for Zephyr memory layout
   - [ ] Verify Zephyr boots in simulation
   - [ ] Check UART console output shows Zephyr banner

2. **Ethernet Simulation**
   - [ ] Add Ethernet controller to Renode platform
   - [ ] Create network bridge in Renode script
   - [ ] Enable host network access for testing
   - [ ] Test TCP connection from host to simulated MCXN947

3. **Python Peripheral Models**
   - [ ] Update `motor_model.py` for CAN-FD Zephyr driver
   - [ ] Update `turret_model.py` for PWM Zephyr driver
   - [ ] Test models with Zephyr firmware

4. **CI/CD Integration**
   - [ ] Update Jenkins pipeline for Zephyr builds
   - [ ] Use `west build` instead of `cmake && make`
   - [ ] Update firmware metrics for Zephyr (expect ~150KB)
   - [ ] Verify Renode simulation stage passes

**Expected Outcome:** Zephyr firmware runs in Renode with Ethernet connectivity

---

## Phase 6: Hardware Validation (Week 7-8)

### Hardware Setup
- [ ] Acquire FRDM-MCXN947 development board ($100)
- [ ] Install Zephyr SDK on development machine
- [ ] Connect Ethernet cable (board to router/switch)
- [ ] Connect USB-UART adapter for console
- [ ] Install J-Link or OpenOCD debugger (optional)

### Flash & Test
1. **Initial Flash with Zephyr**
   - [ ] Build firmware: `west build -b frdm_mcxn947`
   - [ ] Flash via USB: `west flash` (uses MCU-Link debugger on-board)
   - [ ] Open serial terminal: `screen /dev/ttyACM0 115200`
   - [ ] Verify Zephyr banner and boot messages

2. **Ethernet Connectivity Test**
   - [ ] Verify board gets IP: 192.168.1.10
   - [ ] Ping from PC: `ping 192.168.1.10`
   - [ ] Connect to TCP server: `nc 192.168.1.10 5001`
   - [ ] Send test ControlMessage bytes
   - [ ] Verify status UDP broadcasts received

3. **GPIO Test**
   - [ ] Verify LED0 blinks at 1 Hz (heartbeat)
   - [ ] Press emergency stop button, verify ERROR state
   - [ ] LED1 shows Ethernet connection status

4. **CAN-FD Loopback**
   - [ ] Configure CAN controller in loopback mode
   - [ ] Send test frame (ID 0x100, data 0x01020304)
   - [ ] Verify echo received in logs
   - [ ] Measure timing: < 1ms roundtrip

5. **Performance Metrics**
   - [ ] Measure CPU utilization: `kernel profiling` shell command
   - [ ] Check thread stack usage: `kernel stacks`
   - [ ] Monitor network throughput: `net stats`
   - [ ] Verify heap usage: `kernel heap`

6. **Network Protocol Validation**
   - [ ] Create Python AI unit simulator:
     ```python
     import socket
     sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
     sock.connect(('192.168.1.10', 5001))
     ctrl_msg = struct.pack('<4s4f2fBB', b'CTRL', 0.5, 0.5, 0.5, 0.5, 0, 0, 1, 0xAA)
     sock.send(ctrl_msg)
     ```
   - [ ] Verify ControlMessage parsed correctly
   - [ ] Monitor UDP StatusMessage broadcasts with Wireshark
   - [ ] Test reconnection after network disconnect

**Expected Outcome:** Firmware runs on physical hardware, Ethernet communication validated

---

## Phase 7: Documentation & TRL 4 Sign-off (Week 9)

### Documentation Updates
- [ ] Update README.md with Zephyr RTOS and TRL 4 status
- [ ] Update ARCHITECTURE.md: threading model, Zephyr drivers, DeviceTree
- [ ] Create Zephyr setup guide: SDK installation, west workflow
- [ ] Document Ethernet protocol: TCP server, ControlMessage/StatusMessage format
- [ ] Create TRL 4 validation report

### Test Reports
- [ ] Ethernet connectivity test results (TCP, UDP, throughput)
- [ ] Thread performance metrics (CPU, stack, timing)
- [ ] Hardware test results (photos, oscilloscope captures)
- [ ] Network packet captures (Wireshark .pcap files)
- [ ] CAN-FD loopback results

### TRL 4 Validation Checklist
- [ ] Zephyr RTOS integrated and operational
- [ ] Ethernet TCP/IP server functional (192.168.1.10:5001)
- [ ] All Zephyr drivers configured via DeviceTree
- [ ] Multi-threaded architecture implemented (6 threads)
- [ ] Firmware runs on physical FRDM-MCXN947 hardware
- [ ] Network communication validated with Python AI simulator
- [ ] Timing requirements met (1s heartbeat ±10ms)
- [ ] No memory leaks or stack overflows
- [ ] Clean build passes in Jenkins (Zephyr west build)
- [ ] Hardware validation complete with test reports

### Sign-off
- [ ] Review TRL 4 exit criteria against NASA standards
- [ ] Approval from project lead
- [ ] Tag release: `v2.0-trl4-zephyr`
- [ ] Merge to main branch

**Expected Outcome:** TRL 4 complete, ready for TRL 5 (system integration with AI unit)

---

## Success Metrics

### Performance Targets
- **Boot Time:** < 2s from reset to TCP server ready
- **CPU Utilization:** < 30% in IDLE, < 60% in RUNNING
- **Network Latency:** < 20ms TCP roundtrip (AI unit → MCXN947 → response)
- **Throughput:** > 100 KB/s Ethernet (well below 100 Mbps capacity)
- **Timing Accuracy:** ±10ms for 1s heartbeat, ±5ms for 50ms status broadcast
- **Memory Usage:** < 60% Flash (< 1.2MB), < 60% RAM (< 307KB)

### Code Quality
- Zero compiler warnings in Release build
- Zero static analysis errors (cppcheck)
- Zephyr coding style compliance
- All Kconfig options documented

### Network Protocol
- ControlMessage parsing: 100% success rate
- StatusMessage broadcast: 20 Hz (50ms period)
- TCP reconnection: < 1s recovery time
- Checksum validation: 0% false positives/negatives

---

## Risk Mitigation

### Technical Risks
1. **Zephyr Learning Curve**
   - Mitigation: Start with Zephyr samples, read documentation thoroughly
   - Fallback: Community support on Discord/GitHub, extensive examples available

2. **Ethernet Driver Issues**
   - Mitigation: MCXN947 supported in Zephyr mainline, proven driver
   - Fallback: Use USB CDC-ECM as Ethernet-over-USB if PHY issues

3. **Hardware Availability Delay**
   - Mitigation: Order FRDM-MCXN947 immediately ($100, in stock)
   - Fallback: Continue with Renode simulation until hardware arrives

4. **Network Performance**
   - Mitigation: Profile network stack, optimize buffer sizes
   - Fallback: Reduce status broadcast rate if CPU-bound

### Schedule Risks
- **Buffer Time:** 3-week buffer built into 9-week plan
- **Parallel Development:** Simulation and driver config can overlap
- **Early Testing:** Test Ethernet in Week 2-3, not waiting until end
- **Incremental Milestones:** Each phase has clear deliverable

---

## Zephyr vs QP/C++ Comparison (Final)

| Aspect | Zephyr (Selected) | QP/C++ (Rejected) |
|--------|-------------------|-------------------|
| **Ethernet Stack** | Built-in lwIP (1 day) | Manual integration (1 week) |
| **Time to TCP Server** | Week 2-3 | Week 5-6 |
| **Driver Availability** | All drivers in mainline | Must write all BSP code |
| **DeviceTree Config** | Yes (pin config without code) | No (manual register setup) |
| **Code Size** | ~150KB Flash | ~50KB Flash |
| **Determinism** | Good (priority preemption) | Excellent (event-driven) |
| **Learning Curve** | Moderate | Steep |
| **Production Use** | Very common in robotics | Less common |
| **Community Support** | Large (Discord, forums) | Small |
| **TRL 4 Timeline** | 9 weeks | 12 weeks |

**Decision:** Zephyr RTOS is the right choice for Ethernet-based robot architecture.

---

## Next Steps After TRL 4

**TRL 5 Preview:** Subsystem integration and end-to-end validation
- Develop AI unit application (Python/ROS on Raspberry Pi or Pixel)
- Integrate real motor controllers via CAN-FD
- Add GPS module and IMU sensor
- Implement sensor fusion (Kalman filter)
- Path planning with waypoint following
- Vision-based obstacle detection
- End-to-end teleoperation test

**Estimated Timeline:** TRL 5 in Q2 2026

---

## Resources

### Hardware Required
- FRDM-MCXN947 development board (~$100)
- Ethernet cable (Cat5e or better)
- USB-UART adapter (for console) (~$10)
- Oscilloscope (for timing validation - optional)
- CAN transceiver breakout (MCP2551) (~$5 - optional)
- Raspberry Pi or used Pixel 10 Pro (AI unit, TRL 5)

### Software
- Zephyr SDK 0.16.x (free, open source)
- west tool (free, Python package)
- Python 3.8+ (for AI unit simulator)
- Wireshark (network protocol analyzer - free)
- Renode (simulation - free)

### Documentation
- [Zephyr Documentation](https://docs.zephyrproject.org/)
- [Zephyr Networking Guide](https://docs.zephyrproject.org/latest/connectivity/networking/index.html)
- [FRDM-MCXN947 Zephyr Board](https://docs.zephyrproject.org/latest/boards/nxp/frdm_mcxn947/doc/index.html)
- [MCXN947 Reference Manual](https://www.nxp.com/docs/en/reference-manual/MCXN94XRM.pdf)
- [FRDM-MCXN947 User Guide](https://www.nxp.com/docs/en/user-guide/FRDMMCXN947UG.pdf)
- [Zephyr Getting Started](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)

---

**Document Version:** 2.0 (Zephyr RTOS)  
**Last Updated:** December 7, 2025  
**Architecture Decision:** Switched from QP/C++ to Zephyr for Ethernet support  
**Owner:** Kobayashi Maru Project Team
