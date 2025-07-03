#pragma once
#include <Machine.hpp>

struct Settings {
    struct IO {
        using Device = Machine::IO::UART;
        struct Enable {
            static constexpr const bool ASSERT = true;
        };
    };
    struct Timer {
        struct Enable {
            static constexpr const bool ALARM     = true;
            static constexpr const bool SCHEDULER = true;
        };

        static constexpr const unsigned long FREQUENCY = 10'000;
        static constexpr const unsigned long ALARM     = 1'000;
        static constexpr const unsigned long SCHEDULER = 1'000;
    };
};

//template <typename>
//struct Scheduler;
//struct Thread;
//struct RR;
//
//template <typename T>
//struct Traits;
//
//template <>
//struct Traits<Scheduler<Thread>> {
//    typedef RR Criterion;
//};
