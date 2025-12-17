# QP vs MIT/Permissive Alternatives for Embedded Systems

| Feature | QP (Quantum Platform) | TinyFSM | Automaton | FreeRTOS + Event Queues | SObjectizer | Boost MSM |
|---------|---------------------|---------|-----------|------------------------|------------|-----------|
| **License** | GPLv3 / Commercial | MIT | MIT | MIT | MIT | Boost Software License |
| **Event-driven** | ✅ Active Objects | ✅ Events to FSM | ✅ Event-driven FSM | ✅ With queues / notifications | ✅ Message-passing AO | ✅ Event-driven FSM |
| **Hierarchical State Machines (HSM)** | ✅ Full support | ✅ Yes | ✅ Yes | Partial, must implement | ✅ Yes | ✅ Yes |
| **Embedded-friendly / MCU support** | ✅ Very small, runs on MCUs | ✅ Very lightweight | ✅ Arduino / MCU compatible | ✅ Very lightweight | Medium, more desktop-oriented | Medium/High, C++ only, template-heavy |
| **Concurrency / Active Objects** | ✅ Built-in AO + RTOS optional | ❌ Only FSM, no threads | ❌ Only FSM, no threads | ✅ Tasks + queues (user implemented AO) | ✅ Threads + AO | ❌ Only FSM, user must handle threads |
| **Footprint** | Very small (few KB) | Ultra-small | Small | Small | Medium | Medium/High |
| **Tooling / Visual modeling** | ✅ QM modeling tool | ❌ Minimal | ❌ Minimal | ❌ Minimal | ❌ None | ❌ None |
| **Commercial use in proprietary products** | ✅ Only with commercial license | ✅ Free, MIT | ✅ Free, MIT | ✅ Free, MIT | ✅ Free, MIT | ✅ Free, permissive |

---

## Key Takeaways

1. **Closest QP alternative for embedded motion control:**
   - **TinyFSM** → Ultra-lightweight, MIT-licensed, HSM support.
   - **Automaton** → Lightweight, Arduino/MCU ready, MIT-licensed.
   - **FreeRTOS + Event Queues** → Adds Active Object style concurrency, but HSM must be implemented.

2. **Best for industrial use in proprietary firmware:**  
   - Any MIT/permissively licensed framework avoids GPL risks.  
   - **QP free GPL version is a no-go**; commercial QP is expensive and needs negotiation.

3. **Trade-offs:**  
   - QP gives **full AO + HSM framework + tooling** out-of-the-box.  
   - MIT alternatives are **lighter and license-safe**, but may require **more manual integration**, especially for concurrency.
