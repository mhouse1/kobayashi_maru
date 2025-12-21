# TRL 4 Implementation Plan
**Technology Readiness Level 4:** Component and subsystem validation in laboratory environment

**Start Date:** Q2 2026  
**Target Completion:** Q2 2026  
**Previous:** TRL 3 completed – Component validation on physical hardware  
**Architecture:** Zephyr RTOS on FRDM-MCXN947, Ethernet, CAN-FD, GPIO, PWM, ADC fully integrated

---

## TRL 4 Definition

**Goal:** Validate subsystems integrated on actual hardware with functional runtime behavior in a controlled laboratory environment.

**Exit Criteria:**
- All robot subsystems running simultaneously on the MCXN947 board
- TCP/IP server functional and communicating with AI simulator
- GPIO, PWM, CAN-FD, ADC drivers operating correctly in runtime
- System threads (Supervisor, Motor Control, Ethernet, Sensor Fusion, Turret, Path Planner) executing concurrently
- Performance metrics within target ranges
- Clean integration with TRL-3 components and hardware test reports
- Readiness for TRL 5 system stress testing and multi-node integration

---

## Architecture Overview

- **MCXN947 Board:** Single-core Zephyr configuration for TRL-4 runtime validation  
- **Threads:** Multi-threaded architecture from TRL-3 executed concurrently  
- **Subsystems Validated:**
  - CPU, SRAM, Flash
  - GPIO & LEDs
  - UART / Console
  - CAN-FD Motor Modules
  - PWM
  - ADC / Analog Inputs
  - Ethernet / lwIP TCP Server
  - Sensor mock data (IMU, GPS, Cameras)
- **Network:** Local lab network with AI unit simulator
- **Firmware:** Build from TRL-3 Zephyr application

---

## Phase 1: System Initialization & Integration (Week 1)

### Objectives
- Bring up all TRL-3 components on single board
- Verify correct thread scheduling and inter-thread communication

### Tasks
1. Flash latest TRL-3 firmware
2. Confirm UART console output for all threads
3. Validate Supervisor thread coordination
4. Initialize CAN-FD loopback with motors connected
5. Initialize GPIO LEDs and test toggling during runtime
6. Validate ADC readings with dummy or live inputs
7. Run PWM outputs to motors or test load

**Expected Outcome:** All subsystems initialized and running, confirming TRL-4 readiness

---

## Phase 2: Functional Ethernet & TCP/IP Validation (Week 2)

### Objectives
- Test real-time TCP/IP communication between robot and AI simulator

### Tasks
1. Connect Ethernet to lab network
2. Assign static IP: 192.168.1.10
3. Start TCP server on MCXN947
4. Validate connection from AI simulator or PC: `nc 192.168.1.10 5001`
5. Send ControlMessage payloads, verify parsing
6. Broadcast StatusMessage every 50ms via UDP
7. Monitor latency, jitter, and packet loss
8. Capture Wireshark traces for validation

**Expected Outcome:** Functional end-to-end Ethernet communication with correct message handling

---

## Phase 3: Motor Control & CAN-FD Runtime (Week 2-3)

### Objectives
- Validate CAN-FD motor control under normal runtime conditions

### Tasks
1. Configure CAN-FD controller in normal mode
2. Send motor commands from Supervisor thread
3. Measure command-response latency
4. Validate encoder feedback
5. Test PWM control signals
6. Ensure safety interlocks (emergency stop buttons) work

**Expected Outcome:** Motors respond to commands correctly; no thread conflicts or missed messages

---

## Phase 4: GPIO & LED Runtime Validation (Week 3)

### Objectives
- Validate LED and button behavior during system operation

### Tasks
1. Blink LED0 at 1 Hz (heartbeat)
2. LED1 indicates Ethernet connectivity
3. Press emergency stop button → confirm ERROR state
4. Toggle LEDs in response to runtime events

**Expected Outcome:** GPIO pins and LEDs respond correctly in live system

---

## Phase 5: ADC Runtime Testing (Week 3)

### Objectives
- Verify analog input acquisition and scaling during system operation

### Tasks
1. Connect test voltage to ADC channel
2. Read ADC values via Zephyr API in periodic thread
3. Validate voltage conversion and scaling
4. Ensure no missed samples under load

**Expected Outcome:** Accurate, periodic ADC readings during runtime without interference

---

## Phase 6: Renode Simulation Verification (Optional, Week 4)

- Use updated TRL-4 Renode script to verify integration logic before deploying to additional boards
- Confirm Ethernet, CAN-FD, GPIO, PWM, ADC drivers behave as expected in simulation
- Update Python peripheral models for multi-subsystem runtime verification

---

## Phase 7: Performance Metrics & Logging (Week 4)

- CPU utilization per thread: <60% running, <30% idle
- Ethernet latency: <20ms TCP roundtrip
- Heartbeat LED: ±10ms accuracy
- Status message: 20Hz broadcast (±5ms)
- PWM duty cycle and ADC sampling verified against expected values

**Expected Outcome:** System meets runtime performance requirements

---

## Phase 8: Documentation & TRL-4 Sign-off (Week 4)

- Update README.md: TRL-4 runtime integration report
- ARCHITECTURE.md: Include updated threading model, Ethernet, and peripheral behavior
- Test reports: Ethernet logs, CAN-FD timing, ADC measurements, PWM verification
- Capture photos/videos of robot operating in lab
- Tag release: `v0.4.0-trl4-complete`
- Sign-off from project lead

**Expected Outcome:** TRL 4 complete – full subsystem validation on physical hardware in lab environment, ready for TRL 5 system stress and multi-node integration

---

**Resources Needed (TRL 4)**

- **Hardware:**
  - FRDM-MCXN947 development board
  - Ethernet cable and network switch
  - USB-UART adapter
  - Motor controllers (CAN-FD)
  - LED indicators and test loads
  - ADC test voltage sources

- **Software:**
  - Zephyr SDK + west
  - Python 3.8+ for AI simulator
  - Wireshark
  - Renode 1.16.0 (optional simulation)

---

**Document Version:** 0.3.0 (TRL 4 Plan)  
**Last Updated:** December 20, 2025  
**TRL Level:** 4 (Subsystem Functional Runtime Validation)  
**Owner:** Kobayashi Maru Project Team

