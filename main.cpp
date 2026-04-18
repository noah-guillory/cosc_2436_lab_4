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

using namespace std;

const size_t MIN_TELLERS = 1;
const size_t MAX_TELLERS = 5;

// Integer time units.
using Time = int;

// We will be tracking teller state in a variable std::vector.
using TellerIndex = size_t;

// Arrival event containing only the arrival and transaction times.
struct ArrivalEvent {
    Time arrivalTime;
    Time transactionTime;
};

// This is a common idiom in FP, wrapping a type in another to yield better
// semantics (meaning) while gaining some static type checking. This stacking can
// usually be optimized out by the compiler. It could also be a provisional
// placeholder for types that might be expanded later.
struct Customer {
    ArrivalEvent arrivalEvent;
};

// A departure event including the expected departure time and the
// teller being departed from.
struct DepartureEvent {
    Time departureTime;
    TellerIndex tellerIndex;
};

// Either an arrival or departure event. std::variant can be thought of as a type-safe union.
using Event = std::variant<ArrivalEvent, DepartureEvent>;

// Helper function to get the time from either an arrival or departure event.
Time get_event_time(const Event& e) {
    if(holds_alternative<ArrivalEvent>(e)) {
        return get<ArrivalEvent>(e).arrivalTime;
    }
    return get<DepartureEvent>(e).departureTime;
}

// A compare functor / function object for the priority queue.
// TODO: Implement operator() to create a min-heap (earliest event time has highest priority).
//       std::priority_queue is a max-heap by default. Consult its documentation to understand
//       how the comparator affects ordering.
struct CompareEvent {
    bool operator()(const Event & e1, const Event & e2) {
        return true; // Replace this.
    }
};

// TODO: Implement the Teller class.
//
// A teller tracks whether they are currently busy or available, and accumulates
// the total time they have spent working across all customers they serve.
//
// Use std::optional<Time> to represent the busy state:
//   - nullopt means the teller is available
//   - a Time value means the teller started working at that time
//
// Required public interface:
//   Teller()                          - Default constructor. Available, zero accumulated time.
//   bool isAvailable() const          - True if not busy.
//   void startWork(Time currentTime)  - Mark as busy starting at currentTime.
//   void stopWork(Time currentTime)   - Mark as available. Add elapsed time to accumulated total.
//   Time elapsedTimeWorking() const   - Return total accumulated busy time.


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

        // TODO: Load all arrival events from simulationInput into the event priority queue.
    }

    // Sets up the simulation for the given number of tellers.
    void setupSimulation(size_t tellerCount) {
        if(tellerCount < MIN_TELLERS) {
            throw invalid_argument("Teller count must be >= 1");
        }
        if(tellerCount > MAX_TELLERS) {
            throw invalid_argument("Teller count must be <= 5");
        }

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

    // TODO: Implement processArrival.
    //
    // When a customer arrives:
    //   - If a teller is available AND the bank line is empty, the customer goes
    //     directly to that teller. Start the teller working and add a departure
    //     event to the event queue.
    //   - Otherwise, the customer joins the end of the bank line.
    //
    // Use searchAvailableTellers() to find an available teller.
    void processArrival(Time currentTime, const ArrivalEvent& arrivalEvent) {

    }

    // TODO: Implement processDeparture.
    //
    // When a customer departs (finishes their transaction):
    //   - If the bank line is empty, the teller stops working.
    //   - Otherwise, take the next customer from the front of the bank line.
    //     The teller remains busy (do not stop and restart them).
    //     Add a new departure event to the event queue.
    void processDeparture(Time currentTime, const DepartureEvent& departureEvent) {

    }

    // TODO: Implement processEvent.
    //
    // Dispatch to processArrival or processDeparture based on the event type.
    // Use std::holds_alternative and std::get to inspect the variant.
    void processEvent(Time currentTime, const Event& e) {

    }

    // TODO: Implement runSimulation.
    //
    // Process events from the event queue until it is empty.
    // For each iteration: get the top event, remove it, extract its time,
    // and pass it to processEvent.
    void runSimulation() {

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
 * 1 teller(s):  ___
 * 2 teller(s):  ___
 * 3 teller(s):  ___
 * 4 teller(s):  ___
 * 5 teller(s):  ___
 *
 * Q1: At what number of tellers does adding more stop reducing the max busy time?
 *     Why does this happen for this particular input?
 * A1:
 *
 * Scenario 2: Rush Hour
 * 1 teller(s):  ___
 * 2 teller(s):  ___
 * 3 teller(s):  ___
 * 4 teller(s):  ___
 * 5 teller(s):  ___
 *
 * Q2a: Compare the results for 1 teller vs. 3 tellers. What does the difference
 *      tell the bank manager about staffing during peak hours?
 * A2a:
 *
 * Q2b: You may notice that adding a 5th teller actually increases the max busy
 *      time compared to 4 tellers. Why does this happen?
 * A2b:
 *
 * Scenario 3: Steady Trickle
 * 1 teller(s):  ___
 * 2 teller(s):  ___
 * 3 teller(s):  ___
 * 4 teller(s):  ___
 * 5 teller(s):  ___
 *
 * Q3: What do the results tell you about hiring additional tellers for this arrival
 *     pattern? Is there a scenario where a bank could have *too many* tellers?
 * A3:
 *
 * Scenario 4: Morning Rush Then Calm
 * 1 teller(s):  ___
 * 2 teller(s):  ___
 * 3 teller(s):  ___
 * 4 teller(s):  ___
 * 5 teller(s):  ___
 *
 * Q4: How do the results differ from Scenario 2 (pure rush hour)? What does this
 *     suggest about staffing strategies -- should the manager staff for the peak
 *     or the average?
 * A4:
 *
 * Scenario 5: Simultaneous Arrival
 * 1 teller(s):  ___
 * 2 teller(s):  ___
 * 3 teller(s):  ___
 * 4 teller(s):  ___
 * 5 teller(s):  ___
 *
 * Q5: What is the relationship between the number of tellers and the max busy time
 *     for this scenario? Why is this the "worst case" for a queue-based system?
 * A5:
 *
 * Q6 (General): Based on all five scenarios, explain in your own words why a priority
 *     queue is used for the event queue while a regular queue is used for the bank line.
 *     Could you swap them? What would go wrong?
 * A6:
 *
 */
