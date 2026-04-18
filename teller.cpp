//
// Created by noah on 4/18/26.
//

#include "teller.h"

#include <stdexcept>
#include <doctest/doctest.h>

bool Teller::isAvailable() const {
    return !this->busyStartTime.has_value();
}

void Teller::startWork(Time currentTime) {
    if (this->busyStartTime) {
        elapsedBusyTime += currentTime - this->busyStartTime.value();
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

    this->totalBusyTime += elapsedBusyTime + (currentTime - this->busyStartTime.value());
    this->elapsedBusyTime = 0;
    this->busyStartTime = std::nullopt;
}

Time Teller::elapsedTimeWorking() const {
    return this->totalBusyTime;
}

/// UNIT TESTS
TEST_CASE("Teller Class Implementation Tests") {
    // Test setup, initializes a new teller for every subcase.
    Teller teller;

    SUBCASE("Initial teller should be free and no elapsed work time") {
        CHECK(teller.isAvailable());
        CHECK_EQ(teller.elapsedTimeWorking(), 0);
    }

    SUBCASE("Teller when started work should not be available") {
        teller.startWork(0);
        CHECK_FALSE(teller.isAvailable());
    }

    SUBCASE("Should accumulate total busy time when teller starts and stops work") {
        int startTime = 0;
        int endTime = 5;
        CHECK_NOTHROW(teller.startWork(startTime));
        CHECK_NOTHROW(teller.stopWork(endTime));

        CHECK_EQ(teller.elapsedTimeWorking(), endTime - startTime);
    }

    SUBCASE("Teller can start work back to back and all the time is accumulated") {
        int job1Start = 0;
        int job1End = 10;

        int job1Time = job1End - job1Start;

        int job2Start = 10;
        int job2End = 20;
        int job2Time = job2End - job2Start;

        int expectedTotalTime = job1Time + job2Time;

        teller.startWork(job1Start);
        teller.startWork(job2Start);

        teller.stopWork(job2End);

        CHECK_EQ(teller.elapsedTimeWorking(), expectedTotalTime);
    }

    SUBCASE("Should throw exception if stopping work if teller is not busy") {
        CHECK_THROWS(teller.stopWork(0));
    }

    SUBCASE("Should throw exception if stop time is earlier than start time when stopping work") {
        teller.startWork(10);
        CHECK_THROWS(teller.stopWork(5));
    }

    SUBCASE("Accumulates all work time over multiple jobs") {
        int job1Start = 0;
        int job1End = 10;

        int job1Time = job1End - job1Start;

        int job2Start = 15;
        int job2End = 20;

        int job2Time = job2End - job2Start;

        int expectedTotalTime = job1Time + job2Time;

        teller.startWork(job1Start);
        teller.stopWork(job1End);

        teller.startWork(job2Start);
        teller.stopWork(job2End);

        CHECK_EQ(teller.elapsedTimeWorking(), expectedTotalTime);
    }
}