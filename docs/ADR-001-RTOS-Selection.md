# Architecture Decision Record: RTOS Selection for Control MCU

**Status:** Accepted
**Date:** 2025-12-10
**Context:**
The Kobayashi Maru project uses a modular architecture with a high-level AI module (e.g., Google Pixel 10, Raspberry Pi CM5, Jetson) responsible for advanced networking, AI, and user interface tasks. The MCU is dedicated to real-time control (motors, sensors, safety) and communicates with the AI module via CAN-FD or UART.

## Decision
Use FreeRTOS (with optional QP/C++ for event-driven logic) on the MCU, while offloading all advanced networking and high-level features to the AI module running Linux.

## Options Considered
- **FreeRTOS (C or C++):**
  - Lightweight, portable, and highly deterministic.
  - Easy to bring up on new MCUs; broad vendor support.
  - Ideal for real-time, safety-critical control tasks.
- **Zephyr RTOS:**
  - Feature-rich, with built-in networking, device tree, and modular drivers.
  - More complex and heavier; best for MCUs handling advanced networking or multi-threaded features.
- **Bare-metal:**
  - Maximum determinism and minimal footprint.
  - Lacks RTOS features for scheduling, synchronization, and maintainability.

## Rationale
- The AI module is far better suited for advanced networking (Ethernet, Wi-Fi, BLE, cloud, OTA, security) and high-level processing.
- The MCU’s role is focused on deterministic, low-latency control, which FreeRTOS is optimized for.
- FreeRTOS is easier to port and reuse across different MCUs, supporting future hardware changes.
- Zephyr’s advanced features are not needed on the MCU, since those responsibilities are offloaded.

## Consequences
- Simplifies MCU firmware, improves reliability, and eases porting.
- Ensures clear separation of concerns: AI module for high-level tasks, MCU for real-time control.
- If future requirements demand more complex MCU features (networking, security, multi-core), Zephyr can be reconsidered.

## References
- See ARCHITECTURE.md for system partitioning and communication details.
- See TRL3_PLAN.md for implementation phases and hardware/software responsibilities.
