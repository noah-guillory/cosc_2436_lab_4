
//
// Created by noah on 4/18/26.
//

#ifndef LAB4_QUEUES_EVENT_H
#define LAB4_QUEUES_EVENT_H
#include <variant>

#include "teller.h"
#include "time.h"

// A departure event including the expected departure time and the
// teller being departed from.
struct DepartureEvent {
    Time departureTime;
    TellerIndex tellerIndex;
};

// Arrival event containing only the arrival and transaction times.
struct ArrivalEvent {
    Time arrivalTime;
    Time transactionTime;
};

// Either an arrival or departure event. std::variant can be thought of as a type-safe union.
using Event = std::variant<ArrivalEvent, DepartureEvent>;

Time get_event_time(const Event& e);

#endif //LAB4_QUEUES_EVENT_H
