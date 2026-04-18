# BankSim3000 — Lab 4: Queues

**COSC 2436 — Data Structures · Spring 2026**

An event-driven bank teller simulation that helps a branch manager decide how many tellers to hire. The simulation models customers arriving, waiting in a FIFO bank line, and being served by one or more tellers, then measures how hard each teller was worked across five staffing scenarios.

---

## Project Structure

| File | Description |
|---|---|
| `main.cpp` | `BankSim3000` class, event processing logic, scenario runner, and `main()` |
| `teller.h / teller.cpp` | `Teller` class — tracks busy/idle state and accumulated work time |
| `event.h / event.cpp` | `ArrivalEvent`, `DepartureEvent`, and the `Event` variant type |
| `time.h` | `Time` type alias (`int` time units) |
| `logger.h` | `SIM_TRACE` debug-logging macro |
| `test_main.cpp` | doctest entry point for the unit test suite |
| `CMakeLists.txt` | CMake build for both the production binary and the test suite |
| `lab4_queues.md` | Full lab specification and grading rubric |

---

## Key Concepts

- **`std::queue`** — FIFO bank line for waiting customers
- **`std::priority_queue`** — min-heap event queue (earliest event processed first)
- **`std::variant`** — type-safe union dispatching `ArrivalEvent` / `DepartureEvent`
- **`std::optional`** — nullable teller busy-start time
- **Event-driven simulation** — time advances by jumping from event to event

---

## Requirements

- C++23 compiler (GCC 13+, Clang 16+, or MSVC 19.38+)
- CMake 3.25+

---

## Building

```bash
cmake -B build
cmake --build build
```

The production binary is placed at `build/banksim` and the test binary at `build/banksim_tests`.

---

## Running

**Simulation:**
```bash
./build/banksim
```

**Unit tests:**
```bash
./build/banksim_tests
# or via CTest:
ctest --test-dir build
```

---

## Scenarios

The program runs up to five staffing scenarios (1–5 tellers each) and prints the maximum teller busy time for every configuration:

| # | Name | Description |
|---|---|---|
| 1 | Textbook | 4 customers, moderate overlap — matches the textbook example |
| 2 | Rush Hour | 8 back-to-back customers with long transactions |
| 3 | Steady Trickle | 5 customers spaced 20 time units apart |
| 4 | Morning Rush Then Calm | Burst of 4, then sparse arrivals |
| 5 | Simultaneous Arrival | 5 customers all arriving at the same instant |

Analysis answers for each scenario are recorded as a comment block at the bottom of `main.cpp`.

---

## Git History

Commits follow the checkpoints required by the lab spec:

1. Initial setup — starter code added
2. `Teller` class implemented
3. `CompareEvent` comparator and `setupEventQueue` implemented
4. `processArrival`, `processDeparture`, `processEvent`, and `runSimulation` implemented
5. Scenarios run; results and analysis added

---

## License

Academic use only — COSC 2436, Lone Star College.
