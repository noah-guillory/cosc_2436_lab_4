//
// Created by noah on 4/18/26.
//

#ifndef LAB4_QUEUES_TELLER_H
#define LAB4_QUEUES_TELLER_H
#include <cstddef>
#include <optional>

#include "time.h"

constexpr size_t MIN_TELLERS = 1;
constexpr size_t MAX_TELLERS = 5;

// We will be tracking teller state in a variable std::vector.
using TellerIndex = std::size_t;


class Teller {
private:
    std::optional<Time> busyStartTime;
    Time elapsedBusyTime;
    Time totalBusyTime;

public:
    Teller(): busyStartTime(std::nullopt), totalBusyTime(0), elapsedBusyTime(0) {}

    /**
     * Indicates if the given teller is available for work
     * @return true if the teller is available for work
     */
    [[nodiscard]] bool isAvailable() const;

    /**
     * Starts the teller working
     *
     * @pre Teller must be available for work. Throws exception if not
     * @param currentTime The time at which the teller start works
     */
    void startWork(Time currentTime);

    /**
     * Stops the teller's work, adds the work time to their total work time.
     *
     * @pre The teller must currently be busy
     * @param currentTime The time at which the teller stops work
     */
    void stopWork(Time currentTime);

    /**
     *
     * @return total time working
     */
    [[nodiscard]] Time elapsedTimeWorking() const;
};



#endif //LAB4_QUEUES_TELLER_H
