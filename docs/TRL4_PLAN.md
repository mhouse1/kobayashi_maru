# TRL 4 Implementation Plan
**Technology Readiness Level 4:** Component and/or breadboard validation in laboratory environment

**Start Date:** December 7, 2025  
**Target Completion:** Q1 2026  
**Previous:** TRL 3 completed (Build #76, commit `a6051e7`)

---

## TRL 4 Definition (NASA Standard)

**Goal:** Validate individual technology components and/or basic subsystems in a laboratory environment with representative conditions.

**Exit Criteria:**
- Stand-alone component validation complete
- Integration with realistic supporting elements
- Readiness for system-level integration
- Performance measured against requirements

---

## Phase 1: QP/C++ Framework Integration (Week 1-2)

### Objectives
Replace stub implementations with real Quantum Platform framework to enable event-driven architecture.

### Tasks
1. **Download & Integrate QP/C++**
   - [ ] Download QP/C++ v7.x from [Quantum Leaps](https://www.state-machine.com/qpcpp/)
   - [ ] Extract to `firmware/qp/` directory
   - [ ] Review QP ARM Cortex-M port examples
   - [ ] Add QP source files to CMakeLists.txt

2. **CMake Build System Update**
   - [ ] Create `firmware/qp/CMakeLists.txt` for QP library
   - [ ] Add QP include directories to main CMakeLists.txt
   - [ ] Configure QP port for ARM Cortex-M33
   - [ ] Link QP library with main firmware

3. **QP Initialization**
   - [ ] Remove `firmware/src/qp_stubs.cpp`
   - [ ] Implement `QP::QF::init()` in main.cpp
   - [ ] Configure event pool sizes (small: 32B, medium: 64B, large: 128B)
   - [ ] Set up QP priorities for 6 Active Objects
   - [ ] Call `QP::QF::run()` to enter event loop

4. **SysTick Timer Setup**
   - [ ] Configure SysTick for 1ms interrupts (150 MHz / 1000)
   - [ ] Implement `SysTick_Handler()` calling `QP::QF::tickX()`
   - [ ] Verify timing with oscilloscope or logic analyzer

5. **Validation**
   - [ ] Compile firmware with real QP (expect size increase ~10-15KB)
   - [ ] Boot in Renode simulation
   - [ ] Verify QP event loop executes (not stuck in WFI)
   - [ ] Confirm Active Objects initialize

**Expected Outcome:** Firmware uses real QP framework, event loop operational

---

## Phase 2: BSP Driver Implementation (Week 3-5)

### UART Driver (Week 3)
**Purpose:** Debug console output for development and testing

**Implementation:**
- [ ] Study MCXN947 FlexComm UART registers (Reference Manual Ch. 48)
- [ ] Implement `BSP_uartInit(115200, 8N1)` in bsp_drivers.c
- [ ] Implement `BSP_uartPutchar(char c)` - blocking transmit
- [ ] Implement `BSP_uartGetchar()` - non-blocking receive
- [ ] Add newline translation (`\n` → `\r\n`)
- [ ] Update `extern "C"` declarations in bsp.hpp

**Testing:**
- [ ] Renode: verify UART output in telnet console
- [ ] Hardware: connect USB-UART adapter, use PuTTY/screen
- [ ] Print "Kobayashi Maru v1.0 - TRL 4" at boot

### GPIO Driver (Week 3)
**Purpose:** Status LEDs and emergency stop button

**Implementation:**
- [ ] Implement `BSP_gpioInit()` - configure LED pins as outputs
- [ ] Implement `BSP_ledOn(uint8_t ledNum)`, `BSP_ledOff(uint8_t ledNum)`
- [ ] Implement `BSP_gpioReadButton()` - read SW2 emergency stop
- [ ] Configure LED0 (heartbeat), LED1 (error), LED2 (CAN activity)

**Testing:**
- [ ] Renode: verify GPIO state changes in monitor
- [ ] Hardware: confirm LEDs blink, button reads correctly

### Timer Driver (Week 4)
**Purpose:** QP time base and periodic events

**Implementation:**
- [ ] Configure SysTick for 1ms tick (150 MHz CPU)
- [ ] Implement `SysTick_Handler()` ISR calling `QF::tickX(0)`
- [ ] Add ISR priority configuration (ensure below QP critical section)

**Testing:**
- [ ] Verify 1ms timing with oscilloscope
- [ ] Test QTimeEvt fires at correct intervals
- [ ] Validate 1-second heartbeat LED blink

### ADC Driver (Week 4)
**Purpose:** Battery voltage monitoring (future expansion)

**Implementation:**
- [ ] Implement `BSP_adcInit()` - configure ADC channel
- [ ] Implement `BSP_adcRead(uint8_t channel)` - read voltage
- [ ] Add voltage divider scaling (0-30V → 0-3.3V)

**Testing:**
- [ ] Renode: return simulated voltage value
- [ ] Hardware: measure known voltage, verify reading

### PWM Driver (Week 5)
**Purpose:** Motor control signals (placeholder for TRL 5)

**Implementation:**
- [ ] Implement `BSP_pwmInit()` - configure FlexPWM
- [ ] Implement `BSP_pwmSetDuty(uint8_t channel, float duty)`
- [ ] Configure 4 channels for motor control (20 kHz PWM)

**Testing:**
- [ ] Renode: log PWM duty cycle changes
- [ ] Hardware: verify PWM frequency/duty with oscilloscope

### CAN-FD Driver (Week 5)
**Purpose:** Communication with motor modules (loopback test)

**Implementation:**
- [ ] Implement `BSP_canfdInit(500 kbps)` - configure CAN controller
- [ ] Implement `BSP_canfdSend(id, data, len)` - transmit frame
- [ ] Implement `BSP_canfdReceive()` - check RX FIFO
- [ ] Configure loopback mode for testing

**Testing:**
- [ ] Renode: verify CAN frames in Python model
- [ ] Hardware: loopback test (TX → RX, verify echo)

**Expected Outcome:** All BSP drivers functional, hardware interfaces operational

---

## Phase 3: Firmware Refactoring (Week 6)

### Rename Android → Ethernet
**Purpose:** Update code to match Ethernet architecture (consistency with docs)

**Files to Update:**
1. `firmware/src/qp_app/android_comm.cpp` → `ethernet_comm.cpp`
2. `firmware/include/robot.hpp` - signals, enums, class declarations
3. `firmware/src/qp_app/supervisor.cpp` - variable names, handlers
4. All other Active Objects referencing Android

**Changes:**
```cpp
// Before (TRL 3)
class AndroidCommAO { ... };
Priority::ANDROID_COMM
SIG_ANDROID_CMD, SIG_ANDROID_GPS
m_androidConnected

// After (TRL 4)
class EthernetCommAO { ... };
Priority::ETHERNET_COMM
SIG_ETHERNET_CMD, SIG_ETHERNET_GPS
m_ethernetConnected
```

**Testing:**
- [ ] Compile after each rename (incremental changes)
- [ ] Run Jenkins build to verify no breakage
- [ ] Update comments to reflect Ethernet TCP/IP communication

**Expected Outcome:** Code matches architecture documentation

---

## Phase 4: Active Object Implementation (Week 7-8)

### Supervisor AO Enhancement
**Current:** Stub state machine (always IDLE)  
**Target:** Full state machine with transitions

**States:**
- `IDLE` - System powered, waiting for initialization
- `READY` - All subsystems initialized, awaiting start command
- `RUNNING` - Normal operation, monitoring subsystems
- `ERROR` - Fault detected, safe mode active

**Events:**
- `SIG_INIT_COMPLETE` - All AOs initialized
- `SIG_START_CMD` - Begin operation
- `SIG_EMERGENCY_STOP` - Immediate shutdown
- `SIG_HEARTBEAT` - 1-second periodic event

**Implementation:**
- [ ] Implement state transition logic
- [ ] Add 1s heartbeat QTimeEvt
- [ ] Broadcast system state changes
- [ ] Handle emergency stop signal

### Ethernet Comm AO Enhancement
**Current:** Stub UART parsing  
**Target:** Ethernet socket interface (simulated in TRL 4)

**Implementation:**
- [ ] Update comments to Ethernet protocol
- [ ] Define ControlMessage and StatusMessage structs
- [ ] Simulate TCP connection state (DISCONNECTED/CONNECTED)
- [ ] Parse binary protocol (placeholder for TRL 5)

### Motor Control AO
**Implementation:**
- [ ] Receive motor speed commands from Supervisor
- [ ] Set PWM duty cycles via BSP driver
- [ ] Monitor motor status (placeholder)

**Expected Outcome:** Active Objects execute state machines, post events

---

## Phase 5: Simulation Validation (Week 9)

### Renode Testing
- [ ] Firmware boots and prints version string
- [ ] Supervisor transitions: IDLE → READY → RUNNING
- [ ] Heartbeat LED blinks every 1 second
- [ ] UART shows state transitions: "Supervisor: IDLE" → "Supervisor: RUNNING"
- [ ] Emergency stop button triggers ERROR state
- [ ] CAN-FD loopback test passes
- [ ] No crashes or undefined behavior

### Jenkins CI/CD
- [ ] All stages pass (clean build + cached build)
- [ ] Firmware size tracked (~15-20KB expected with real QP)
- [ ] Renode simulation logs show expected output
- [ ] No compiler warnings in Release build

**Expected Outcome:** Firmware validated in simulation environment

---

## Phase 6: Hardware Validation (Week 10-11)

### Hardware Setup
- [ ] Acquire FRDM-MCXN947 development board
- [ ] Install J-Link or OpenOCD debugger
- [ ] Connect USB-UART adapter to FlexComm pins
- [ ] Connect CAN transceiver (optional, for loopback)

### Flash & Test
1. **Initial Flash**
   - [ ] Build firmware in Release mode
   - [ ] Flash via OpenOCD: `openocd -f board/frdmcxn947.cfg -c "program firmware.elf verify reset exit"`
   - [ ] Verify boot via UART console

2. **UART Console Test**
   - [ ] Open serial terminal (115200 8N1)
   - [ ] Verify boot message: "Kobayashi Maru v1.0 - TRL 4"
   - [ ] Confirm state transitions printed

3. **GPIO Test**
   - [ ] Verify LED0 blinks at 1 Hz (heartbeat)
   - [ ] Press SW2, verify emergency stop triggers ERROR state
   - [ ] LED1 illuminates in ERROR state

4. **Timing Validation**
   - [ ] Connect oscilloscope to LED0 pin
   - [ ] Measure blink period: 1000ms ±1ms
   - [ ] Verify SysTick accuracy

5. **CAN-FD Loopback**
   - [ ] Configure CAN controller in loopback mode
   - [ ] Send test frame (ID 0x100, data 0x01020304)
   - [ ] Verify echo received (LED2 blinks on activity)

6. **Performance Metrics**
   - [ ] Measure CPU utilization (idle vs active)
   - [ ] Verify stack usage (no overflow)
   - [ ] Check heap usage (QP event pools)

**Expected Outcome:** Firmware runs on physical hardware, all components validated

---

## Phase 7: Documentation & TRL 4 Sign-off (Week 12)

### Documentation Updates
- [ ] Update README.md with TRL 4 status
- [ ] Document BSP driver APIs in ARCHITECTURE.md
- [ ] Add hardware setup guide (FRDM-MCXN947 connections)
- [ ] Create TRL 4 validation report

### Test Reports
- [ ] Component test results (each BSP driver)
- [ ] Simulation test results (Renode logs)
- [ ] Hardware test results (oscilloscope captures, photos)
- [ ] Performance metrics (CPU, memory, timing)

### TRL 4 Validation Checklist
- [ ] All BSP drivers implemented and tested
- [ ] QP/C++ framework integrated and operational
- [ ] Active Objects execute state machines
- [ ] Firmware runs on physical hardware
- [ ] Timing requirements met (1ms SysTick, 1s heartbeat)
- [ ] No memory leaks or stack overflows
- [ ] Clean build passes in Jenkins
- [ ] Hardware validation complete

### Sign-off
- [ ] Review TRL 4 exit criteria
- [ ] Approval from project lead
- [ ] Tag release: `v1.0-trl4`
- [ ] Merge to main branch

**Expected Outcome:** TRL 4 complete, ready for TRL 5 (system integration)

---

## Success Metrics

### Performance Targets
- **Boot Time:** < 500ms from reset to RUNNING state
- **CPU Utilization:** < 20% in IDLE, < 50% in RUNNING
- **Timing Accuracy:** ±1% for 1ms SysTick, ±10ms for 1s heartbeat
- **Memory Usage:** < 50% Flash (< 1MB), < 50% RAM (< 256KB)

### Code Quality
- Zero compiler warnings in Release build
- Zero static analysis errors (cppcheck)
- All unit tests pass (if added)
- Consistent coding style (embedded C++ best practices)

---

## Risk Mitigation

### Technical Risks
1. **QP Framework Integration Issues**
   - Mitigation: Start with QP examples, incremental integration
   - Fallback: Use simpler RTOS (FreeRTOS) if QP proves problematic

2. **Hardware Availability Delay**
   - Mitigation: Order FRDM-MCXN947 immediately
   - Fallback: Continue with Renode simulation, defer hardware to Phase 6

3. **BSP Driver Complexity**
   - Mitigation: Use MCUXpresso SDK as reference
   - Fallback: Simplified drivers for TRL 4, full implementation in TRL 5

### Schedule Risks
- **Buffer Time:** 2-week buffer built into 12-week plan
- **Parallel Tasks:** Simulation and BSP development can overlap
- **Incremental Testing:** Test each component immediately after implementation

---

## Next Steps After TRL 4

**TRL 5 Preview:** System integration with Ethernet communication
- Implement TCP/IP stack (lwIP) on MCXN947
- Develop AI unit application (Python/ROS on Raspberry Pi)
- Integrate motor controllers via CAN-FD
- Add GPS/IMU sensor fusion
- Implement path planning and obstacle avoidance

**Estimated Timeline:** TRL 5 in Q2 2026

---

## Resources

### Hardware Required
- FRDM-MCXN947 development board (~$100)
- J-Link debugger or OpenOCD-compatible adapter (~$20-400)
- USB-UART adapter (FTDI, CP2102) (~$10)
- Oscilloscope (for timing validation)
- CAN transceiver (MCP2551 or similar) (~$5)

### Software
- QP/C++ Framework (free, open source)
- MCUXpresso SDK (NXP, free)
- OpenOCD (free, open source)
- Renode (free, open source)
- ARM GCC Toolchain (free)

### Documentation
- [QP/C++ Documentation](https://www.state-machine.com/qpcpp/)
- [MCXN947 Reference Manual](https://www.nxp.com/docs/en/reference-manual/MCXN94XRM.pdf)
- [FRDM-MCXN947 User Guide](https://www.nxp.com/docs/en/user-guide/FRDMMCXN947UG.pdf)
- [ARM Cortex-M33 Technical Reference](https://developer.arm.com/documentation/100230/latest/)

---

**Document Version:** 1.0  
**Last Updated:** December 7, 2025  
**Owner:** Kobayashi Maru Project Team
