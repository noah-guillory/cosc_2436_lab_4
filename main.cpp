// BankSim3000
//
// The purpose of this bank and teller simulation is to help a bank manager to make an informed
// decision on how many tellers to hire at a branch with longer than desired wait times.

#include <iostream>
#include <variant>
#include <vector>
#include <queue>
#include <cassert>
#include <algorithm>
#include <optional>

#include "event.h"
#include "logger.h"

using namespace std;

// This is a common idiom in FP, wrapping a type in another to yield better
// semantics (meaning) while gaining some static type checking. This stacking can
// usually be optimized out by the compiler. It could also be a provisional
// placeholder for types that might be expanded later.
struct Customer {
    ArrivalEvent arrivalEvent;
};

// A compare functor / function object for the priority queue.
//       std::priority_queue is a max-heap by default. Consult its documentation to understand
//       how the comparator affects ordering.
struct CompareEvent {
    bool operator()(const Event & e1, const Event & e2) const {
        return get_event_time(e1) > get_event_time(e2);
    }
};

// Holds the simulation results for a single run.
struct SimulationResults {
    vector<Time> elapsedTimeBusy;

    // Finds the max teller time which is perhaps proportional to the customer wait time.
    Time maxTellerBusyTime() {
        return *max_element(elapsedTimeBusy.begin(), elapsedTimeBusy.end());
    }

    SimulationResults(vector<Time> elapsedTimeBusy) : elapsedTimeBusy(elapsedTimeBusy) { }
};

// A line of customers waiting to be served by a teller.
using BankLine = queue<Customer>;
// The event priority queue used by the simulation.
using EventQueue = priority_queue<Event, vector<Event>, CompareEvent>;
// A list of arrival events used to start the simulation.
using SimulationInput = vector<ArrivalEvent>;

class BankSim3000 {
private:
    // Input is stored locally to help restart the simulation for multiple teller counts.
    SimulationInput simulationInput;
    // The event queue. Initially this is loaded with the simulation input.
    EventQueue eventQueue;
    // The bank line. Initially this is empty.
    BankLine bankLine;
    // One teller simulation state for each teller.
    vector<Teller> tellers;

    // Resets the tellers vector to the requested count, each default-constructed.
    void resetTellers(size_t tellerCount) {
        tellers.assign(tellerCount, Teller{});
    }

    // Clears the bank line.
    void clearBankLine() {
        assert(bankLine.empty());
        while(!bankLine.empty()) {
            bankLine.pop();
        }
    }

    // Clears the event queue and initializes it with our input data.
    void setupEventQueue() {
        assert(eventQueue.empty());
        while(!eventQueue.empty()) {
            eventQueue.pop();
        }

        for (const auto &event: simulationInput) {
            eventQueue.push(event);
        }
    }

    // Sets up the simulation for the given number of tellers.
    void setupSimulation(size_t tellerCount) {
        if(tellerCount < MIN_TELLERS) {
            throw invalid_argument("Teller count must be >= 1");
        }
        if(tellerCount > MAX_TELLERS) {
            throw invalid_argument("Teller count must be <= 5");
        }

        SIM_TRACE("--- Starting Simulation with " << tellerCount << " Teller(s) ---");

        setupEventQueue();
        resetTellers(tellerCount);
        clearBankLine();
    }

    // Returns the index of an available teller, or nullopt if all are busy.
    optional<size_t> searchAvailableTellers() {
        for(size_t i = 0; i < tellers.size(); ++i) {
            if(tellers[i].isAvailable()) {
                return i;
            }
        }
        return nullopt;
    }

    // When a customer arrives:
    //   - If a teller is available AND the bank line is empty, the customer goes
    //     directly to that teller. Start the teller working and add a departure
    //     event to the event queue.
    //   - Otherwise, the customer joins the end of the bank line.
    //
    // Use searchAvailableTellers() to find an available teller.
    void processArrival(Time currentTime, const ArrivalEvent& arrivalEvent) {
        SIM_TRACE("[Time " << currentTime << "] Customer ARRIVED (Transaction time: " << arrivalEvent.transactionTime << ")");
        const auto maybeTellerIndex = searchAvailableTellers();
        const auto isLineEmpty = bankLine.size() == 0;

        if (maybeTellerIndex && isLineEmpty) {
            const auto tellerIndex = maybeTellerIndex.value();
            auto& teller = tellers[tellerIndex];

            teller.startWork(currentTime);
            Time newDepartureTime = currentTime + arrivalEvent.transactionTime;
            eventQueue.push(DepartureEvent(newDepartureTime, tellerIndex));

            SIM_TRACE("           -> Sent directly to Teller " << tellerIndex);
        } else {
            bankLine.push(Customer(arrivalEvent));
            SIM_TRACE("           -> No tellers available. Added to line. (Line size: " << bankLine.size() << ")");
        }

    }

    // When a customer departs (finishes their transaction):
    //   - If the bank line is empty, the teller stops working.
    //   - Otherwise, take the next customer from the front of the bank line.
    //     The teller remains busy (do not stop and restart them).
    //     Add a new departure event to the event queue.
    void processDeparture(Time currentTime, const DepartureEvent& departureEvent) {
        SIM_TRACE("[Time " << currentTime << "] Customer DEPARTED from Teller " << departureEvent.tellerIndex);
        auto& teller = tellers[departureEvent.tellerIndex];

        teller.stopWork(currentTime);

        if (bankLine.size() == 0) {
            SIM_TRACE("           -> Line is empty. Teller " << departureEvent.tellerIndex << " goes idle.");
            return;
        }

        auto nextCustomer = bankLine.front();
        bankLine.pop();

        SIM_TRACE("           -> Teller " << departureEvent.tellerIndex
                  << " pulled next customer from line. (Remaining in line: " << bankLine.size() << ")");


        teller.startWork(currentTime);

        Time newDepartureTime = currentTime + nextCustomer.arrivalEvent.transactionTime;
        eventQueue.push(DepartureEvent(newDepartureTime, departureEvent.tellerIndex));
    }

    // Dispatch to processArrival or processDeparture based on the event type.
    // Use std::holds_alternative and std::get to inspect the variant.
    void processEvent(const Time currentTime, const Event& e) {
        if (std::holds_alternative<ArrivalEvent>(e)) {
            processArrival(currentTime, std::get<ArrivalEvent>(e));
        } else {
            processDeparture(currentTime, std::get<DepartureEvent>(e));
        }
    }

    //
    // Process events from the event queue until it is empty.
    // For each iteration: get the top event, remove it, extract its time,
    // and pass it to processEvent.
    void runSimulation() {
        while (!eventQueue.empty()) {
            auto nextEvent = eventQueue.top();
            eventQueue.pop();
            const auto eventTime = get_event_time(nextEvent);
            processEvent(eventTime, nextEvent);
        }
        SIM_TRACE("--- Simulation Complete ---\n");
    }

    // Gathers teller busy times into a SimulationResults struct.
    // std::transform is like map in functional languages — it applies a function
    // to each element of a range and stores the results.
    SimulationResults gatherResults() {
        vector<Time> elapsedTimeBusy(tellers.size());
        transform(tellers.begin(), tellers.end(), elapsedTimeBusy.begin(), [](auto teller) {
            return teller.elapsedTimeWorking();
        });
        return SimulationResults{elapsedTimeBusy};
    }

public:
    BankSim3000(SimulationInput simulationInput) : simulationInput(simulationInput) { }

    Time maxTellerBusyTime(size_t tellerCount) {
        setupSimulation(tellerCount);
        runSimulation();
        return gatherResults().maxTellerBusyTime();
    }
};

// Helper to run a scenario and print results for 1-5 tellers.
void runScenario(const string& name, SimulationInput input) {
    BankSim3000 sim(input);
    cout << "=== " << name << " ===" << endl;
    for(size_t t = MIN_TELLERS; t <= MAX_TELLERS; ++t) {
        cout << "  " << t << " teller(s): " << sim.maxTellerBusyTime(t) << endl;
    }
    cout << endl;
}

int main() {

    // Scenario 1: Textbook
    // Four customers with moderate overlap.
    runScenario("Scenario 1: Textbook",
        {{20, 6}, {22, 4}, {23, 2}, {30, 3}});

    // Scenario 2: Rush Hour
    // Eight customers arriving nearly back-to-back with long transactions.
    runScenario("Scenario 2: Rush Hour",
        {{1, 8}, {2, 6}, {3, 9}, {4, 5}, {5, 7}, {6, 10}, {7, 4}, {8, 6}});

    // Scenario 3: Steady Trickle
    // Five customers spaced far apart with short transactions.
    runScenario("Scenario 3: Steady Trickle",
        {{10, 2}, {30, 3}, {50, 1}, {70, 2}, {90, 3}});

    // Scenario 4: Morning Rush Then Calm
    // A burst of four customers, then sparse arrivals.
    runScenario("Scenario 4: Morning Rush Then Calm",
        {{1, 5}, {2, 7}, {3, 4}, {4, 6}, {30, 2}, {50, 3}, {70, 1}});

    // Scenario 5: Simultaneous Arrival
    // Five customers all arriving at the exact same time with identical transactions.
    runScenario("Scenario 5: Simultaneous Arrival",
        {{10, 5}, {10, 5}, {10, 5}, {10, 5}, {10, 5}});

    return 0;
}

/*
 * SCENARIO RESULTS AND ANALYSIS
 * =============================
 *
 * Scenario 1: Textbook
 * 1 teller(s):  15
 * 2 teller(s):  11
 * 3 teller(s):  9
 * 4 teller(s):  9
 * 5 teller(s):  9
 *
 * Q1: At what number of tellers does adding more stop reducing the max busy time?
 *     Why does this happen for this particular input?
 * A1: Adding more tellers stops reducing the max busy time at 3 tellers. With 3 tellers,
 *     all four customers go directly to a teller upon arrival without ever waiting in line,
 *     so there is no queuing backlog to eliminate. The 4th customer arrives at t=30
 *     after all tellers have gone idle and goes to Teller 0 each time. A 4th or
 *     5th teller sits completely idle for the entire simulation and cannot reduce the
 *     workload of the busiest teller.
 *
 * Scenario 2: Rush Hour
 * 1 teller(s):  55
 * 2 teller(s):  30
 * 3 teller(s):  21
 * 4 teller(s):  15
 * 5 teller(s):  16
 *
 * Q2a: Compare the results for 1 teller vs. 3 tellers. What does the difference
 *      tell the bank manager about staffing during peak hours?
 * A2a: With 1 teller the max busy time is 55 minutes; with 3 tellers it drops to 21, about
 *      a 62% reduction. This tells the manager that during peak hours, even a modest
 *      increase in staffing reduces the load on any single teller. The long
 *      queue that builds up under 1 teller forces that teller to work continuously serving
 *      all 8 customers back-to-back, whereas 3 tellers split that backlog so no single
 *      teller shoulders more than about one-third of the total work.
 *
 * Q2b: You may have noticed that in the Rush Hour scenario, adding a 5th teller doesn't always reduce the max busy
 *      time compared to 4 tellers. Let's walk through why.
 *      Customer F arrives at t=6 with a 10-minute transaction — far longer than anyone else (4–9 min). Whichever
 *      teller ends up serving F will dominate the max busy time, because F alone adds 10 minutes to that teller's
 *      total. So the real question is: which teller picks up F?
 *
 *      4-teller run — what happens step by step:
 *      Time 1-4: Customers A, B, C, D arrive. Tellers 0, 1, 2, 3 each take one. All busy.
 *      Time 5-8: E, F, G, H arrive. No tellers free. Line: [E, F, G, H].
 *      Time 8: Teller 1 finishes B (6 min job). Pulls E from front of line. F is now second in line.
 *      Time 9: Teller 0 finishes A — has worked 8 min so far. Teller 3 finishes D — has worked 5 min so far.
 *              Both free at the same instant. F is at the front of the line; one of them grabs it.
 *      At t=9, depending on which teller grabs F:
 *              If T0 grabs F: T0 total = 8 + 10 = 18 min
 *              If T3 grabs F: T3 total = 5 + 10 = 15 min
 * Look at your program's output for the 4-teller run.
 * Which of these two values did your simulation produce, and therefore which teller grabbed F on your system?
 * This is a common problem with task scheduling where sometimes adding resources can actually make the problem worse.
 * See Braess' paradox for a similar problem with traffic networks.
 * https://en.wikipedia.org/wiki/Braess%27s_paradox
 *
 * A2b: The simulation produced 15 for the 4-teller run, meaning Teller 3 grabbed Customer F.
 *      At t=9, both Departure(9,0) and Departure(9,3) are in the priority queue with equal
 *      timestamps. The heap processes Departure(9,3) first (it was inserted later and rose to
 *      the top due to heap structure), so Teller 3 departs first and immediately pulls F from
 *      the front of the line (T3 total = 5 + 10 = 15). When Teller 0's departure is then
 *      processed, it pulls G instead (T0 total = 8 + 4 = 12). Adding a 5th teller makes
 *      things worse because E is served directly by Teller 4 instead of waiting in line,
 *      which frees Teller 1 (only 6 min of prior work) to grab F at t=8, giving
 *      T1 total = 6 + 10 = 16 > 15.
 *
 * Scenario 3: Steady Trickle
 * 1 teller(s):  11
 * 2 teller(s):  11
 * 3 teller(s):  11
 * 4 teller(s):  11
 * 5 teller(s):  11
 *
 * Q3: What do the results tell you about hiring additional tellers for this arrival
 *     pattern? Is there a scenario where a bank could have *too many* tellers?
 * A3: The max busy time is identical (11) for every teller count. Customers arrive every
 *     20 minutes with transactions of 1–3 minutes, so each customer always finds Teller 0
 *     idle on arrival and goes directly to service with no waiting. Extra tellers never
 *     receive a single customer, sitting idle all day. Yes, a bank can absolutely
 *     have too many tellers. If arrivals are sparse enough that one teller is never
 *     backlogged, additional tellers add payroll cost with zero benefit to customers or
 *     to the max busy metric.
 *
 * Scenario 4: Morning Rush Then Calm
 * 1 teller(s):  28
 * 2 teller(s):  15
 * 3 teller(s):  17
 * 4 teller(s):  11
 * 5 teller(s):  11
 *
 * Q4: How do the results differ from Scenario 2 (pure rush hour)? What does this
 *     suggest about staffing strategies -- should the manager staff for the peak
 *     or the average?
 * A4: Unlike Scenario 2, where adding tellers incrementally reduces max busy time,
 *     Scenario 4 is non-incremental: going from 2 to 3 tellers actually increases max
 *     busy time (15 → 17). This happens because the sparse afternoon customers always
 *     land on Teller 0 (the first available teller), and with 3 tellers the morning
 *     rush is distributed so that Teller 0 ends up idle at t=12, earlier than with
 *     2 tellers, accumulating all three afternoon transactions on top of a heavier
 *     morning load. The results suggest the manager should not simply staff for the
 *     peak: the right staffing level depends on the full day's pattern. Here, 4 tellers
 *     (matching the burst size) is optimal; staffing for the calm period (1–2 tellers)
 *     creates an unacceptable morning backlog, but overstaffing (3 tellers) can
 *     ironically worsen the outcome due to scheduling.
 *
 * Scenario 5: Simultaneous Arrival
 * 1 teller(s):  25
 * 2 teller(s):  15
 * 3 teller(s):  10
 * 4 teller(s):  10
 * 5 teller(s):  5
 *
 * Q5: What is the relationship between the number of tellers and the max busy time
 *     for this scenario? Why is this the "worst case" for a queue-based system?
 * A5: Max busy time = ceil(5 / tellers) * 5. With 1 teller: 5*5=25; 2 tellers: 3*5=15;
 *     3 tellers: 2*5=10; 4 tellers: ceil(5/4)*5=2*5=10 (no improvement because 5 customers
 *     can't be split evenly among 4 tellers); 5 tellers: 1*5=5. This is the worst case for
 *     a queue-based system because all customers arrive simultaneously, producing maximum
 *     instantaneous congestion — the queue must absorb all 5 customers at once with no
 *     natural spacing. In every other scenario some customers arrive after earlier ones
 *     have already finished, naturally reducing the backlog. Here the queue grows to its
 *     maximum possible size at time zero and tellers must drain it from scratch, so the
 *     bank line (rather than arrival spacing) entirely determines throughput.
 *
 * Q6 (General): Based on all five scenarios, explain in your own words why a priority
 *     queue is used for the event queue while a regular queue is used for the bank line.
 *     Could you swap them? What would go wrong?
 * A6: The event queue uses a priority queue because events are inserted out of chronological
 *     order — a DepartureEvent created at time t is added to the queue mid-simulation and
 *     must be processed at its scheduled future time, not at the tail end. A priority queue
 *     always surfaces the earliest-time event regardless of insertion order, keeping the
 *     simulation's virtual clock advancing correctly.
 *     The bank line uses a regular FIFO queue because customers must be served in arrival
 *     order — first come, first served is both fair and the real-world model being simulated.
 *     Swapping them would break both structures: if the event queue were a FIFO queue,
 *     departure events inserted mid-simulation would be processed after all pre-loaded
 *     arrival events regardless of time, scrambling the chronological order and corrupting
 *     simulation state. If the bank line were a priority queue, you'd need an arbitrary
 *     priority criterion; choosing arrival time would still give FIFO behavior, but any
 *     other criterion (e.g., shorter transaction first) would change the simulation's
 *     semantics entirely and no longer model a standard bank queue.
 *
 */
