## What is QP (Quantum Platform)?

QP (Quantum Platform) is an event-driven embedded framework designed for real-time systems. It provides:

- **Active Objects (AOs):** Each component has its own thread and event queue.
- **Hierarchical State Machines (HSMs):** Organizes behavior into clear, modular states.
- **Run-to-completion semantics:** Ensures deterministic, non-reentrant event processing.
- **Event-driven architecture:** Inputs are handled as events, improving modularity and portability.
- **RTOS/OS integration:** Typically layered on top of an RTOS or POSIX for simulation.

QP is used to simplify complex embedded software, making systems easier to reason about, test, and port across hardware and operating systems.

# Reusability and Portability: QP vs Non-QP Architectures

This document explains why QP-based designs tend to be more reusable and portable than non-QP architectures, with nuances for context.

---

## 1️⃣ Why QP Boosts Reusability and Portability

QP enforces **strong architecture rules**:

- **Active Objects (AO):** Encapsulate state and behavior, making each component self-contained.
- **Hierarchical State Machines (HSMs):** Clearly defined behavior hierarchy makes logic modular.
- **Event-driven design:** Inputs are abstracted as events, so the same state machine can be reused on different platforms or with different peripherals.
- **Run-to-completion semantics:** Guarantees deterministic behavior, which simplifies moving logic between MCUs, RTOSes, or even POSIX simulations (like Renode).

**Result:** The same AO/HSM can often be deployed across different hardware or RTOS targets with minimal changes.

---

## 2️⃣ Why Non-QP Architectures Are Often Less Reusable

In traditional task-based or flat FSM designs:

- **Tasks and flags:** Logic is often intertwined with a specific RTOS, interrupt setup, or hardware.
- **Global/shared state:** Harder to isolate, making modular reuse risky.
- **Hard-coded timing or priorities:** Transitioning to a different MCU or RTOS may require re-tuning.
- **Scattered state logic:** Event handling is often distributed across ISRs and tasks, so moving the logic requires rewriting multiple pieces.

**Result:** Porting to a new system usually requires **manual adaptation**, and reuse is limited without significant refactoring.

---

## 3️⃣ Nuance

- A **well-disciplined RTOS + modular architecture** can still be portable, but it takes **more effort and discipline** than a QP-style AO/HSM design.
- QP doesn’t magically make code reusable; it **enforces patterns that naturally lead to reuse**.

---

## ✅ Summary

> Saying that “companies not using QP often have setups that are less reusable/portable” is **accurate as a general statement**, because non-QP designs often mix tasks, flags, and global state, whereas QP enforces encapsulation, event-driven behavior, and hierarchy that **naturally promotes portability and reuse**.