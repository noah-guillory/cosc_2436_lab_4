//
// Created by Noah Guillory on 4/18/26.
//

#include "event.h"

Time get_event_time(const Event& e) {
    if(std::holds_alternative<ArrivalEvent>(e)) {
        return std::get<ArrivalEvent>(e).arrivalTime;
    }
    return std::get<DepartureEvent>(e).departureTime;
}