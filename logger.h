//
// Created by noah on 4/18/26.
//

#ifndef LAB4_QUEUES_LOGGER_H
#define LAB4_QUEUES_LOGGER_H

#include <iostream>
#include <cstdlib>
#include <string_view>

namespace Logger {
    inline bool isTraceEnabled() {
        // Evaluated exactly once on the first call
        static const bool enabled = []() {
            if (const char* env_p = std::getenv("BANKSIM_TRACE")) {
                return std::string_view(env_p) == "1";
            }
            return false;
        }();
        return enabled;
    }
}

// A handy macro that allows stream-like logging if the trace is enabled.
#define SIM_TRACE(msg) \
do { \
if (Logger::isTraceEnabled()) { \
std::cout << msg << "\n"; \
} \
} while (false)

#endif //LAB4_QUEUES_LOGGER_H
