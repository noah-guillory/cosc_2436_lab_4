//
// Created by noah on 4/18/26.
//

#include "teller.h"

#include <stdexcept>

bool Teller::isAvailable() const {
    return !this->busyStartTime.has_value();
}

void Teller::startWork(Time currentTime) {
    if (!this->isAvailable()) {
        throw std::logic_error("Teller is not available to start work");
    }

    this->busyStartTime = currentTime;
}

void Teller::stopWork(Time currentTime) {
    if (this->isAvailable()) {
        throw std::logic_error("Teller is not working");
    }

    if (currentTime < this->busyStartTime.value()) {
        throw std::logic_error("The time at which the teller stops work cannot be earlier than the time they started work");
    }

    this->totalBusyTime += this->busyStartTime.value() - currentTime;
    this->busyStartTime = std::nullopt;
}

Time Teller::elapsedTimeWorking() const {
    return this->totalBusyTime;
}
