# Traceability Matrix

| Requirement ID | Sub-Requirement ID | Description                              | Design Reference         | Implementation File(s)         | Test Case(s) / Validation      | Status      |
|:--------------:|:------------------:|:-----------------------------------------|:------------------------|:-------------------------------|:-------------------------------|:------------|
| REQ-001        | REQ-001.1          | Define TCP/IP protocol and message format | ARCHITECTURE.md, system_block_diagram.puml | firmware/src/ethernet_comm.c   | Protocol spec, code review      | Complete    |
|                | REQ-001.2          | Implement TCP server/client               | ARCHITECTURE.md         | firmware/src/ethernet_comm.c   | TRL3_PLAN.md, Jenkins #76      | In Progress |
|                | REQ-001.3          | Validate communication on hardware        | ARCHITECTURE.md         | firmware/src/ethernet_comm.c   | Hardware test report           | Planned     |
|                | REQ-001.1a         | TCP/IP protocol preliminarily defined (initial message format) | ARCHITECTURE.md, system_block_diagram.puml | firmware/src/ethernet_comm.c   | Initial protocol spec, code review | In Progress |
|                | REQ-001.1b         | TCP/IP protocol revised after integration feedback             | ARCHITECTURE.md, system_block_diagram.puml | firmware/src/ethernet_comm.c   | Design review, integration test    | Planned     |
|                | REQ-001.1c         | TCP/IP protocol finalized and documented                      | ARCHITECTURE.md, system_block_diagram.puml | firmware/src/ethernet_comm.c   | Final protocol spec, code review   | Planned     |
| REQ-002        | REQ-002.1          | CAN-FD bus architecture defined           | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Design review                  | Complete    |
|                | REQ-002.2          | CAN-FD driver implemented                 | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Code review                    | In Progress |
|                | REQ-002.3          | CAN-FD loopback test on hardware          | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | CAN-FD loopback test           | Planned     |
|                | REQ-002.1a         | CAN-FD bus architecture preliminarily defined (initial topology, node IDs) | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Initial design review          | In Progress |
|                | REQ-002.1b         | CAN-FD bus architecture revised after hardware feedback                    | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Design review, hardware test   | Planned     |
|                | REQ-002.1c         | CAN-FD bus architecture finalized and documented                           | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Final design review            | Planned     |
| REQ-003        | REQ-003.1          | Emergency stop requirement defined        | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Spec review                    | Complete    |
|                | REQ-003.2          | GPIO pin assigned and schematic updated   | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Schematic review               | In Progress |
|                | REQ-003.3          | Emergency stop code implemented           | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Code review                    | Planned     |
|                | REQ-003.4          | Emergency stop tested on hardware         | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | GPIO test, TRL3_PLAN.md        | Planned     |
|                | REQ-003.1a         | Emergency stop requirement preliminarily defined               | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Initial spec review                | In Progress |
|                | REQ-003.1b         | Emergency stop requirement revised after hardware feedback     | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Design review, hardware test       | Planned     |
|                | REQ-003.1c         | Emergency stop requirement finalized and documented            | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Final spec review                  | Planned     |
| REQ-004        | REQ-004.1          | Modular AI unit interface defined         | ARCHITECTURE.md, README | N/A (external app)             | Protocol spec                  | Complete    |
|                | REQ-004.2          | Pixel/CM5/Jetson integration supported    | ARCHITECTURE.md, README | N/A (external app)             | Integration test               | Planned     |
|                | REQ-004.1a         | Modular AI unit interface preliminarily defined                | ARCHITECTURE.md, README | N/A (external app)             | Initial protocol spec              | In Progress |
|                | REQ-004.1b         | Modular AI unit interface revised after integration feedback   | ARCHITECTURE.md, README | N/A (external app)             | Design review, integration test    | Planned     |
|                | REQ-004.1c         | Modular AI unit interface finalized and documented             | ARCHITECTURE.md, README | N/A (external app)             | Final protocol spec                | Planned     |
| REQ-005        | REQ-005.1          | Turret control protocol defined           | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Spec review                    | Planned    |
|                | REQ-005.2          | Turret control code implemented           | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Code review                    | In Progress |
|                | REQ-005.3          | Turret tested on hardware                 | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Turret test, TRL3_PLAN.md      | Planned     |
|                | REQ-005.1a         | Turret control protocol preliminarily defined                  | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Initial spec review                | In Progress |
|                | REQ-005.1b         | Turret control protocol revised after hardware feedback        | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Design review, hardware test       | Planned     |
|                | REQ-005.1c         | Turret control protocol finalized and documented               | ARCHITECTURE.md         | firmware/src/turret_ctrl.cpp   | Final spec review                  | Planned     |
| REQ-006        | REQ-006.1          | Renode simulation platform defined        | ARCHITECTURE.md, README | simulation/renode/robot_simulation.resc | Design review                  | Complete    |
|                | REQ-006.2          | Simulation scripts implemented            | ARCHITECTURE.md, README | simulation/renode/robot_simulation.resc | Jenkins pipeline, TRL2 checklist | Complete    |
|                | REQ-006.1a         | Renode simulation platform preliminarily defined               | ARCHITECTURE.md, README | simulation/renode/robot_simulation.resc | Initial design review              | In Progress |
|                | REQ-006.1b         | Renode simulation platform revised after integration feedback  | ARCHITECTURE.md, README | simulation/renode/robot_simulation.resc | Design review, integration test    | Planned     |
|                | REQ-006.1c         | Renode simulation platform finalized and documented            | ARCHITECTURE.md, README | simulation/renode/robot_simulation.resc | Final design review                | Planned     |
| REQ-007        | REQ-007.1          | Hardware validation checklist created     | TRL3_PLAN.md            | docs/TRL3_PLAN.md              | Checklist review                | Complete    |
|                | REQ-007.2          | Hardware validation tests executed        | TRL3_PLAN.md            | docs/TRL3_PLAN.md              | Hardware test reports           | Planned     |
|                | REQ-007.1a         | Hardware validation checklist preliminarily defined            | TRL3_PLAN.md            | docs/TRL3_PLAN.md              | Initial checklist review            | In Progress |
|                | REQ-007.1b         | Hardware validation checklist revised after test feedback      | TRL3_PLAN.md            | docs/TRL3_PLAN.md              | Design review, hardware test        | Planned     |
|                | REQ-007.1c         | Hardware validation checklist finalized and documented         | TRL3_PLAN.md            | docs/TRL3_PLAN.md              | Final checklist review              | Planned     |
| REQ-008        | REQ-008.1          | Power supply validation (all rails, brownout/reset)           | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Power test report                | Planned     |
| REQ-009        | REQ-009.1          | Clock source validation (crystal/oscillator, system clock)    | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Clock test report                | Planned     |
| REQ-010        | REQ-010.1          | Bootloader/startup code validation (firmware update, vector table, memory remap) | ARCHITECTURE.md | firmware/src/startup_mcxn947.c | Bootloader test report | Planned     |
| REQ-011        | REQ-011.1          | GPIO/LED bring-up (test all user/status LEDs, validate GPIOs) | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | GPIO/LED test report             | Planned     |
| REQ-012        | REQ-012.1          | EEPROM/Flash storage validation (if present)                  | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Storage test report               | Planned     |
| REQ-013        | REQ-013.1          | Safety features: watchdog timer setup and validation          | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Watchdog test report              | Planned     |
| REQ-014        | REQ-014.1          | Peripheral loopback/self-test routines (CAN, UART, SPI, I2C, PWM, ADC) | ARCHITECTURE.md | firmware/src/bsp/bsp_drivers.c | Peripheral self-test report | Planned     |
| REQ-015        | REQ-015.1          | Thermal/environmental checks (temperature sensor validation)  | ARCHITECTURE.md         | firmware/src/bsp/bsp_drivers.c | Thermal test report               | Planned     |
| REQ-016        | REQ-016.1          | Documentation and code/schematic review before hardware testing | ARCHITECTURE.md, README | docs/TRL3_PLAN.md              | Review checklist                  | Planned     |
| REQ-017        | REQ-017.1          | Automated test scripts for hardware bring-up                  | ARCHITECTURE.md, README | scripts/, docs/TRL3_PLAN.md     | Test script results               | Planned     |
| REQ-018        | REQ-018.1          | RTOS/platform selection and partitioning analysis documented | ARCHITECTURE.md, ADR-001-RTOS-Selection.md | N/A (design decision) | ADR review, architecture review | Complete    |

*Add more requirements and sub-requirements as the project evolves. Update status and references as validation progresses.*

