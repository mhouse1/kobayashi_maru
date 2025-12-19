# QP/C 4.4.01 vs Latest (8.1.1)

QP/C **4.4.01 is very old** (released March 23, 2012). The **latest QP/C version (8.1.1 as of Oct 2025)** represents a *new generation* of the framework. This comparison focuses on **architectural and practical differences** relevant to real embedded systems.

---

## Big Picture Summary

| Area | QP/C 4.4.01 | Latest QP/C (8.1.1) |

|----|----|----|
| Design philosophy | HSMs + events, manual | **Model-driven, contract-based** |
| Active Objects | Heavy, static | **Leaner, scalable AOs** |
| Event memory | Manual pools | **Safer event ownership rules** |
| Time events | Basic | **More deterministic & flexible** |
| RTOS integration | Limited | **First-class FreeRTOS, Zephyr, POSIX** |
| Tooling | Hand-coded | **QM (model-driven codegen)** |
| Safety | Informal | **MISRA-C, defensive runtime checks** |
| C language style | C89-era | **Modernized, C99-friendly** |
| Debug & test | Minimal | **QS tracing & test probes** |

> **Note:** Latest QP/C version referenced here is 8.1.1 (released October 2025). Check the [official release notes](https://github.com/QuantumLeaps/qpc/releases) or [revision history](https://www.state-machine.com/qpc/history.html) for updates.


---

## 1. Architecture & Design Philosophy

### QP/C 4.4.01
- Harel State Machines (HSMs) already present
- Manual wiring and configuration
- Weak enforcement of correctness
- Easy to misuse APIs
- Assumed expert-only usage

### Latest QP/C (8.1.1)
- Explicit enforcement of:
  - Run-to-completion semantics
  - Event ownership rules
  - Clear Active Object boundaries
- Design-by-contract approach
- Far fewer opportunities for race conditions

**This is the most significant change across versions.**

---

## 2. Active Objects (AOs)

### QP/C 4.4.01
- Statically configured and relatively heavy
- Manual stack and queue sizing
- Limited support for priority inversion handling

### Latest QP/C (8.1.1)
- Leaner Active Object implementation
- Clean separation between framework, BSP, and application
- Improved RTOS priority mapping
- Better idle and background processing hooks

**Better suited for modular systems (e.g., CAN-FD nodes, robotics subsystems).**

---

## 3. Event System & Memory Management

### QP/C 4.4.01
- Event pools existed but required careful manual handling
- Weak runtime validation
- Easy to leak, double-free, or mis-size events

### Latest QP/C (8.1.1)
- Stronger event ownership rules
- Optional runtime checks
- Cleaner and safer APIs
- Improved alignment with safety standards

**A major improvement for long-running systems.**

---

## 4. Time Events & Determinism

### QP/C 4.4.01
- Basic time events
- Limited configurability
- Less flexible tick handling

### Latest QP/C (8.1.1)
- Multiple time event rates
- Decoupled tick handling
- Support for low-power and tickless RTOS modes

**Important for battery-powered and real-time robotic systems.**

---

## 5. RTOS Integration

### QP/C 4.4.01
- Dated and limited RTOS ports
- Primitive FreeRTOS support
- No Zephyr support
- POSIX support largely DIY

### Latest QP/C (8.1.1)
- Official, maintained ports for:
  - FreeRTOS
  - Zephyr
  - POSIX (Linux simulation)
- Clean abstraction layers
- Well-suited for Renode and CI-based simulation

**A night-and-day difference for modern workflows.**

---

## 6. Tooling & Model-Driven Development

### QP/C 4.4.01
- Entirely hand-coded
- Diagrams used only for documentation

### Latest QP/C (8.1.1)
- Tight integration with **QM (Quantum Modeler)**
- Visual state machine design
- Auto-generated, correct-by-construction C code
- Prevention of illegal state constructs

**Highly valuable for complex behavioral logic.**

---

## 7. Tracing, Debugging & Testability

### QP/C 4.4.01
- Minimal runtime visibility
- Debugging relied on breakpoints and logs

### Latest QP/C (8.1.1)
- **QS (Quantum Spy)** tracing:
  - Event flow
  - State transitions
  - Timing analysis
- Test probes for deterministic testing and fault injection

**Makes system-level testing practical, especially under simulation.**

---

## 8. Safety, MISRA & Production Readiness

### QP/C 4.4.01
- No explicit safety positioning
- MISRA compliance left to the user

### Latest QP/C (8.1.1)
- MISRA-C alignment
- Defensive programming built-in
- Widely used in industrial, medical, and safety-critical systems

---

## Should You Use QP/C 4.4.01 Today?

**Only if:**
- Maintaining legacy code
- Licensing constraints prevent upgrading
- Extremely resource-constrained targets

**Otherwise:**

> There is no architectural reason to choose QP/C 4.4.01 for new designs.

---

## Opinionated Take (Modern Embedded Systems)

For modern systems involving:
- Modular architectures (e.g., CAN-FD)
- Simulation (Renode, POSIX)
- CI-driven testing

**Latest QP/C (8.1.1) with FreeRTOS or POSIX is the correct architectural choice.**

It enables:
- Early host-based simulation
- Seamless migration to MCU targets
- Deterministic behavior validation

---

## Migration Notes: QP/C 4.4.01 → Latest (8.1.1)

This section highlights **what breaks, what changes, and what must be redesigned** when migrating legacy QP/C 4.4.01 code to QP/C 8.1.1.

---


### 1. API Incompatibilities (Will Break)

- Many public APIs have been **renamed, refactored, or split**
- Function signatures for:
  - Active Object start/stop
  - Event posting
  - Time event arming
  have changed
- Macros and typedefs from 4.4.01 may no longer exist

**Action:** Expect **compile-time failures**; this is normal and intentional.

---

### 2. Active Object Initialization Model

**4.4.01:**
- Manual AO construction
- Application controlled stacks, queues, and priorities


**8.1.1:**
- AO lifecycle is more strictly defined
- Clear separation of:
  - Framework init
  - BSP init
  - Application start

**What breaks:**
- Custom AO startup code
- Direct access to AO internals

**Action:** Refactor AO creation to follow the modern startup sequence.

---

### 3. Event Allocation & Ownership Rules

**4.4.01:**
- Events could be passed and reused informally
- Ownership rules mostly implicit


**8.1.1:**
- Events have **strict ownership semantics**
- Misuse may trigger runtime assertions

**What breaks:**
- Reusing events across AOs
- Storing pointers to events beyond their lifecycle

**Action:** Audit all event flows and ensure one clear owner at a time.

---

### 4. Time Events & Ticks

**4.4.01:**
- Single tick rate
- Direct tick handling common


**8.1.1:**
- Multiple tick rates supported
- Tick source is abstracted

**What breaks:**
- Direct tick ISR coupling
- Assumptions about global tick frequency

**Action:** Move tick handling into the BSP layer and use framework APIs only.

---

### 5. Port Layer & RTOS Integration

**4.4.01:**
- RTOS ports often modified locally
- Weak separation between port and app code


**8.1.1:**
- RTOS ports are **first-class and maintained**
- Strict porting layer boundaries

**What breaks:**
- Custom RTOS hooks inside application code
- Direct OS calls bypassing QP

**Action:** Consolidate OS interactions inside the official QP port layer.

---

### 6. Assertions, Defensive Checks & MISRA

**4.4.01:**
- Minimal runtime checking


**8.1.1:**
- Extensive assertions enabled by default
- MISRA-oriented constraints

**What breaks:**
- Code that previously "worked" but violated framework rules

**Action:** Treat assertion failures as **design bugs**, not framework issues.

---

### 7. State Machine Structure Changes

**4.4.01:**
- Hand-written state handlers
- Looser structure allowed


**8.1.1:**
- Stricter state hierarchy expectations
- Strong alignment with QM-generated code

**What breaks:**
- Non-standard transition patterns
- Custom hacks inside state handlers

**Action:** Refactor toward canonical HSM patterns or regenerate using QM.

---

### 8. Build System & File Layout

**4.4.01:**
- Flat directory layouts common
- Manual Makefiles


**8.1.1:**
- Structured directory layout
- Clear separation of:
  - qpc
  - ports
  - bsp
  - application

**What breaks:**
- Include paths
- Custom build assumptions

**Action:** Reorganize the project structure before debugging logic issues.

---

### Recommended Migration Strategy

1. **Freeze behavior** on 4.4.01 (tests or traces)
2. Upgrade compiler and toolchain first
3. Port to latest QP/C **without changing behavior**
4. Fix compile errors systematically
5. Enable assertions and QS tracing early
6. Only then refactor architecture for improvements

---

### Migration Effort (Rule of Thumb)

| Codebase Size | Expected Effort |
|----|----|
| Small (1–2 AOs) | 1–3 days |
| Medium (5–10 AOs) | 1–2 weeks |
| Large / safety-critical | Multi-week, staged migration |

---

*End of document*

---

## Further Reading & References

- [QP/C Official Website](https://www.state-machine.com/qpc)
- [QP/C Documentation Portal](https://www.state-machine.com/doc/)
- [QP/C Release Notes (GitHub)](https://github.com/QuantumLeaps/qpc/releases)
- [QM Model-Based Design Tool](https://www.state-machine.com/qm)
- [MISRA Compliance in QP](https://www.state-machine.com/doc/AN_QP_MISRA.pdf)

For the most up-to-date information, always refer to the official Quantum Leaps website and GitHub repository.

