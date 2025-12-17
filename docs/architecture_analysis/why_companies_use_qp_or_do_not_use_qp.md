# Why Companies Use QP — and Why They Often Don’t

This document explains **why some companies adopt QP (Quantum Platform)** and **why many do not**, from a technical, organizational, and risk-management perspective. It is written to reflect **real industrial decision-making**, not marketing claims.

---

## What QP Is (Context)

QP is an **event-driven embedded framework** built around:
- Hierarchical State Machines (HSMs)
- Active Objects (one thread/event queue per behavioral component)
- Run-to-completion semantics

QP is typically layered **on top of an RTOS** (or POSIX for simulation).

---

## Why Companies *Choose* QP

### 1️⃣ Architectural Clarity for Complex Behavior

Companies adopt QP when products have:
- Many operating modes
- Complex fault handling
- Strong behavioral coupling to events

QP provides:
- Explicit states and transitions
- Centralized behavior logic
- Elimination of flag-based logic

**Result:** systems that are easier to reason about and review.

---

### 2️⃣ Determinism and Run-to-Completion Guarantees

QP enforces:
- One event processed at a time per Active Object
- No reentrancy inside state machines
- Deterministic execution order

This is valuable in:
- Safety-adjacent systems
- Control systems
- Robotics and motion platforms

---

### 3️⃣ Excellent Fit for Event-Driven Systems

QP maps naturally to systems where inputs are:
- Messages (CAN, UART, Ethernet)
- Timers and watchdogs
- Hardware faults

QP turns these into **first-class events**, rather than scattered ISR logic.

---

### 4️⃣ Testability and Simulation

Companies value QP when they need:
- Host-based simulation (POSIX)
- Deterministic replay of event sequences
- Fine-grained tracing (QS)

This supports:
- CI pipelines
- Early architectural validation
- Safer refactoring

---

### 5️⃣ Strong Architects, Smaller Teams

QP adoption is common when:
- A strong software architect is present
- The team is small to mid-sized
- Architectural consistency matters

In these environments, QP becomes a **force multiplier**.

---

## Why Companies *Do Not* Use QP

### 1️⃣ Organizational Risk Aversion

QP is:
- Technically strong
- But less mainstream than FreeRTOS + tasks

Large organizations often prefer:
- Widely adopted patterns
- Multiple vendor options
- Easier hiring pipelines

**Result:** QP is seen as "non-standard" risk.

---

### 2️⃣ Skill Distribution and Training Cost

Most embedded engineers know:
- RTOS tasks
- Queues and semaphores
- Basic FSMs

Far fewer engineers are fluent in:
- HSM theory
- Event ownership models
- Active Object architectures

Training cost is a real barrier.

---

### 3️⃣ Legacy Code and Inertia

Many companies already have:
- Decades of task-based architectures
- Internal frameworks
- Certified or validated designs

Rewriting to QP would:
- Introduce risk
- Require re-certification
- Delay delivery

---

### 4️⃣ Toolchain and Certification Constraints

Some domains prioritize:
- AUTOSAR
- IEC 61131 / PLC standards
- Model-based tools like Simulink or SCADE

In these environments, QP is often unnecessary or incompatible.

---

### 5️⃣ Perceived Overhead

QP introduces:
- Framework concepts
- Event queues
- Architectural rules

For small or simple products, teams may view this as unnecessary complexity.

---

## Where QP Is Most Commonly Used

QP adoption is most likely in:
- Medical devices
- Aerospace subsystems
- Robotics platforms
- Advanced industrial controllers

These domains value:
- Determinism
- Explicit behavior modeling
- Long-term maintainability

---

## Where QP Is Rarely Used

QP is uncommon in:
- Simple IoT devices
- PLC-dominated systems
- AUTOSAR-centric automotive ECUs
- Extremely cost-optimized consumer electronics

---

## Comparison: QP vs Typical Industrial Architecture

| Aspect | QP-Based | Typical RTOS-Based |
|----|----|----|
| Behavior modeling | Explicit HSMs | Implicit flags/tasks |
| Determinism | Enforced | Depends on discipline |
| Testability | High | Medium–low |
| Learning curve | Steep | Familiar |
| Organizational risk | Higher | Lower |

---

## Interview-Ready Summary

> "Most companies standardize on an RTOS and internal frameworks. QP tends to be adopted selectively in systems with complex reactive behavior, where determinism, testability, and architectural clarity outweigh the cost of training and perceived risk. It’s usually an architect-driven decision, not a company-wide default."

---

## Bottom Line

- QP is **not mainstream**, but it is **highly effective**
- Companies choose QP for **architecture**, not convenience
- Companies avoid QP due to **risk, familiarity, and inertia**
- Knowing *when* QP fits matters more than advocating it everywhere

---

## When I Would Recommend QP (Decision Checklist)

Use this checklist to decide whether QP is a **good architectural fit** for a given product or subsystem. QP is most successful when *multiple* of the following are true.

---

### ✅ Product & System Characteristics

Recommend QP if:
- ☐ The system is **event-driven** (messages, interrupts, timers dominate)
- ☐ The product has **many operating modes** (Init, Operational, Degraded, Error, etc.)
- ☐ Fault handling and recovery logic is **non-trivial**
- ☐ Behavior is easier to describe as **states and transitions** than algorithms
- ☐ Deterministic behavior is more important than raw throughput

Avoid QP if:
- ☐ The system is mostly linear or batch-oriented
- ☐ There are very few modes or transitions

---

### ✅ Concurrency & Architecture

Recommend QP if:
- ☐ You want **clear ownership of state** (one AO owns one behavior)
- ☐ Shared-memory concurrency bugs are a concern
- ☐ Run-to-completion semantics simplify reasoning
- ☐ You prefer message passing over shared flags

Avoid QP if:
- ☐ The design relies heavily on shared global state
- ☐ Tight coupling between tasks is unavoidable

---

### ✅ Team & Organization

Recommend QP if:
- ☐ There is a **software architect** guiding structure
- ☐ The team is small to medium-sized
- ☐ Consistency and long-term maintainability matter
- ☐ The team is open to learning HSM concepts

Avoid QP if:
- ☐ The team is very large and highly distributed
- ☐ Rapid onboarding of junior engineers is the top priority

---

### ✅ Tooling, Testing & Simulation

Recommend QP if:
- ☐ Host-based simulation (POSIX) is valuable
- ☐ Deterministic testing and replay are required
- ☐ CI-based system testing is planned
- ☐ Tracing and runtime visibility are important

Avoid QP if:
- ☐ Debugging is done only via JTAG and printf
- ☐ There is no appetite for architectural testing

---

### ✅ Domain Fit

QP is a strong fit for:
- ☐ Robotics
- ☐ Medical devices
- ☐ Aerospace subsystems
- ☐ Advanced industrial controllers
- ☐ Modular CAN / fieldbus nodes

QP is usually a poor fit for:
- ☐ PLC-centric systems
- ☐ AUTOSAR-dominated automotive ECUs
- ☐ Very small IoT or disposable devices

---

## When QP Failed / Warning Signs

This section captures **real-world scenarios where QP adoption struggles or fails**, and the warning signs that usually appear early. These are **organizational and architectural failures**, not framework defects.

---

### 🚩 1. QP Introduced Without Architectural Ownership

**Symptoms:**
- No clear architect responsible for enforcing patterns
- Each developer uses QP differently
- State machines devolve into large switch statements

**Why it fails:**
- QP amplifies both good and bad architecture
- Without guidance, complexity moves *into* the framework

**Mitigation:**
- Assign a clear architectural owner
- Define AO and event design rules up front

---

### 🚩 2. Task-Based Thinking Forced Into QP

**Symptoms:**
- One Active Object per former RTOS task
- Blocking calls inside state handlers
- Semaphores used inside HSM logic

**Why it fails:**
- Violates run-to-completion semantics
- Destroys determinism

**Mitigation:**
- Redesign behavior as events, not threads
- Keep blocking APIs out of state machines

---

### 🚩 3. Overusing QP for Simple Problems

**Symptoms:**
- HSMs with only 1–2 states
- Complex framework setup for trivial devices

**Why it fails:**
- Perceived as unnecessary complexity
- Team resentment toward the framework

**Mitigation:**
- Use QP selectively
- Keep simple components simple

---

### 🚩 4. Poor Event Design

**Symptoms:**
- Huge, generic events used everywhere
- Events carry excessive data
- State machines inspect payloads to decide meaning

**Why it fails:**
- Weak event semantics
- Loss of clarity and testability

**Mitigation:**
- Design events as *intent*, not data blobs
- Prefer many small, specific signals

---

### 🚩 5. Ignoring Hierarchy Benefits

**Symptoms:**
- Flat state machines
- Duplicated logic across states

**Why it fails:**
- Loses the primary benefit of HSMs
- Code becomes harder to maintain than FSMs

**Mitigation:**
- Actively refactor toward meaningful superstates

---

### 🚩 6. Resistance from the Team

**Symptoms:**
- Engineers bypass QP APIs
- "QP vs RTOS" debates dominate discussions
- Framework blamed for unrelated issues

**Why it fails:**
- Cultural rejection outweighs technical benefits

**Mitigation:**
- Introduce QP incrementally
- Demonstrate value through testing and simulation

---

### 🚩 7. Misaligned Domain or Certification Constraints

**Symptoms:**
- Project requires AUTOSAR, IEC 61131, or PLC-style logic
- Certification bodies expect different tooling

**Why it fails:**
- QP solves a different class of problem

**Mitigation:**
- Use domain-native frameworks instead

---

### 🚩 8. No Investment in Tooling and Testing

**Symptoms:**
- QS tracing disabled or unused
- No host-based simulation
- Debugging relies solely on printf/JTAG

**Why it fails:**
- Benefits of QP are never realized

**Mitigation:**
- Treat tooling as part of the architecture

---

## Summary: Why QP Failures Happen

QP failures are almost always due to:
- Organizational misalignment
- Architectural misuse
- Cultural resistance

Not because of limitations in HSM theory or the framework itself.

---

### 🚦 Rule of Thumb

> If you can draw the system as a **state hierarchy with clear event flows**, and you care about determinism and testability, QP is likely a good choice.

If the system is better described as a **pipeline of tasks**, QP is probably unnecessary.

---

### Interview-Friendly One-Liner

> "I recommend QP selectively—when the system is highly event-driven, mode-rich, and architecturally complex enough that explicit state modeling and run-to-completion semantics reduce risk more than they add learning cost."

---

*End of document*

