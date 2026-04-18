# Lab 4: Queues — BankSim3000

COSC 2436 — Spring 2026

## Introduction

In this lab you will build a bank teller simulation based on the event-driven approach described in Chapter 13.4 of your textbook. The simulation helps a bank manager decide how many tellers to hire at a branch with longer-than-desired wait times.

You are given starter code with the data types, class shell, and several utility methods already written. Your job is to implement the core simulation logic: the `Teller` class, the event comparator, event processing, and the simulation loop. You will then run multiple scenarios and analyze the results.

### What You'll Practice

- `std::queue` as a FIFO bank line
- `std::priority_queue` as a min-heap event queue
- `std::variant` for type-safe event dispatch
- `std::optional` for representing nullable state
- Event-driven simulation design

### C++ Standard

This lab requires C++ 23. Make sure your compiler is set to use this version.

A `CMakeLists.txt` is provided that works with CLion, Visual Studio, and command-line builds. To build from the command line:

```bash
cmake -B build
cmake --build build
```

The executable will be in the `build` directory.

## Git Requirement

This lab requires the use of git. You will submit the entire repository as a zip file. Your commit history is part of the grade — it should demonstrate incremental progress, not a single commit with the finished product.

### Getting Started

```bash
mkdir lab4_queues
cd lab4_queues
git init
```

Copy `main_template.cpp`, `CMakeLists.txt`, and `.gitignore` into the directory. Rename the template to `main.cpp`. Then make your initial commit:

```bash
cp /path/to/main_template.cpp main.cpp
cp /path/to/CMakeLists.txt .
cp /path/to/.gitignore .
git add main.cpp CMakeLists.txt .gitignore
git commit -m "Initial commit: add starter code"
```

### Required Commits

You must commit after completing each of the following steps. You are free (and encouraged) to commit more often than this — these are the minimum checkpoints.

1. **Initial setup** — Starter code added to repo
2. **Teller class** — `Teller` class implemented using `std::optional`
3. **Event queue setup** — `CompareEvent` comparator and `setupEventQueue` implemented
4. **Event processing and simulation loop** — `processArrival`, `processDeparture`, `processEvent`, and `runSimulation` implemented
5. **Scenarios and analysis** — All scenarios run, results and answers added as comments

### Submission

Zip the entire project directory (including the `.git` folder) and submit. We will inspect your commit history.

```bash
cd ..
zip -r lab4_queues.zip lab4_queues/
```

## Part 1: Implementation

Open `main.cpp`. You will find `TODO` markers at each location where you need to write code. Read the surrounding comments and the descriptions below carefully.

**Note:** The starter code will not compile until you complete Step 1 (Teller class). The `BankSim3000` class references `Teller`, so you must implement it first before you can build and test anything.

### Step 1: Teller Class

Implement the `Teller` class. A teller needs to track:

- Whether they are currently busy or available
- When they started their current work (if busy)
- Total accumulated time spent working

Use `std::optional<Time>` to represent the busy state. When the teller is available, the optional holds `nullopt`. When the teller starts working, store the current time. When they stop, compute the elapsed time and accumulate it.

Your class must provide these public methods:

| Method | Description |
|---|---|
| `Teller()` | Default constructor. Teller starts available with zero accumulated time. |
| `bool isAvailable() const` | Returns true if the teller is not currently busy. |
| `void startWork(Time currentTime)` | Marks the teller as busy starting at `currentTime`. |
| `void stopWork(Time currentTime)` | Marks the teller as available. Computes elapsed time since work started and adds it to the accumulated total. |
| `Time elapsedTimeWorking() const` | Returns total accumulated busy time. |

**Commit after completing this step.**

### Step 2: Event Queue Setup

Implement two pieces:

**CompareEvent::operator()** — The event priority queue needs to be a min-heap (earliest events first). `std::priority_queue` is a max-heap by default. Your comparator must reverse the ordering so that the event with the smallest time has the highest priority. Use the provided `get_event_time` helper.

**setupEventQueue()** — Load all arrival events from `simulationInput` into the event priority queue. Since `ArrivalEvent` is one of the types in the `Event` variant, you can push it directly.

**Commit after completing this step.**

### Step 3: Event Processing and Simulation Loop

Implement four methods:

**processArrival(Time currentTime, const ArrivalEvent& arrivalEvent)**

When a customer arrives:
- If a teller is available AND the bank line is empty, the customer goes directly to that teller. Start the teller working and add a departure event to the event queue. The departure time is `currentTime + transactionTime`.
- Otherwise, the customer joins the end of the bank line.

**processDeparture(Time currentTime, const DepartureEvent& departureEvent)**

When a customer departs (finishes their transaction):
- If the bank line is empty, the teller stops working.
- Otherwise, take the next customer from the front of the bank line. The teller remains busy (do not stop and restart them) and a new departure event is added to the event queue with departure time `currentTime + transactionTime`.

**processEvent(Time currentTime, const Event& e)**

Dispatch an event to the correct handler. Use `std::holds_alternative` and `std::get` to determine whether the event is an `ArrivalEvent` or `DepartureEvent` and call the appropriate method.

**runSimulation()**

Process events until the event queue is empty. For each iteration: remove the top event, get its time, and pass it to `processEvent`.

**Commit after completing this step.**

## Part 2: Scenarios and Analysis

The `main()` function runs five scenarios, each with 1 through 5 tellers. After you have a working simulation, compile and run the program. Record your results and answer the questions below.

Add your results and answers as comments at the bottom of `main.cpp` in the following format:

```cpp
/*
 * SCENARIO RESULTS AND ANALYSIS
 * =============================
 *
 * Scenario 1: Textbook
 * 1 teller:  ___
 * 2 tellers: ___
 * ...
 *
 * Q: ...
 * A: ...
 */
```

### Scenario 1: Textbook

```
Input: {20, 6}, {22, 4}, {23, 2}, {30, 3}
```

Four customers with moderate overlap. This is the example from the textbook.

**Q1:** At what number of tellers does adding more stop reducing the max busy time? Why does this happen for this particular input?

### Scenario 2: Rush Hour

```
Input: {1, 8}, {2, 6}, {3, 9}, {4, 5}, {5, 7}, {6, 10}, {7, 4}, {8, 6}
```

Eight customers arriving nearly back-to-back with long transactions.

**Q2a:** Compare the results for 1 teller vs. 3 tellers. What does the difference tell the bank manager about staffing during peak hours?

**Q2b:** You may notice that adding a 5th teller actually *increases* the max busy time compared to 4 tellers. Why does this happen? (Hint: think about how tellers are assigned to customers and how that affects workload distribution.)

### Scenario 3: Steady Trickle

```
Input: {10, 2}, {30, 3}, {50, 1}, {70, 2}, {90, 3}
```

Five customers spaced 20 time units apart with short transactions.

**Q3:** What do the results tell you about hiring additional tellers for this arrival pattern? Is there a scenario where a bank could have *too many* tellers?

### Scenario 4: Morning Rush Then Calm

```
Input: {1, 5}, {2, 7}, {3, 4}, {4, 6}, {30, 2}, {50, 3}, {70, 1}
```

A burst of four customers in the first four time units, then sparse arrivals later.

**Q4:** How do the results differ from Scenario 2 (pure rush hour)? What does this suggest about staffing strategies — should the manager staff for the peak or the average?

### Scenario 5: Simultaneous Arrival

```
Input: {10, 5}, {10, 5}, {10, 5}, {10, 5}, {10, 5}
```

Five customers all arriving at the exact same time with identical transaction lengths.

**Q5:** What is the relationship between the number of tellers and the max busy time for this scenario? Why is this the "worst case" for a queue-based system?

**Q6 (General):** Based on all five scenarios, explain in your own words why a priority queue is used for the event queue while a regular queue is used for the bank line. Could you swap them? What would go wrong?

**Commit after completing this step.**

## Grading

| Component | Weight |
|---|---|
| Teller class (correct use of `std::optional`) | 15% |
| CompareEvent comparator | 10% |
| setupEventQueue | 5% |
| processArrival | 15% |
| processDeparture | 15% |
| processEvent (variant dispatch) | 10% |
| runSimulation (event loop) | 5% |
| Scenario results (correct output) | 10% |
| Analysis questions (thoughtful answers) | 10% |
| Git history (incremental commits) | 5% |
