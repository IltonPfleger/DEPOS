#pragma once

struct Thread;
template <typename T>
struct RR;
template <typename T>
struct RateMonotonic;

struct Traits {
    struct Machine {
        static constexpr const char *NAME = "sifive_u";
        static constexpr const int XLEN   = 64;
        static constexpr const int CPUS   = 5;
        static constexpr const int BSP    = 1;
    };

    struct System {
        static constexpr const int MULTITASK = true;
    };

    struct Timer {
        static constexpr bool Enable             = true;
        static constexpr unsigned long MHz       = 1'000'000;
        static constexpr unsigned long Frequency = MHz;
    };

    struct Alarm {
        static constexpr const bool Enable             = true;
        static constexpr const unsigned long Frequency = Timer::MHz;
    };

    struct Memory {
        static constexpr const unsigned int ORDER = 30;
        static constexpr const unsigned int SIZE  = (1 << ORDER);
        struct Page {
            static constexpr const unsigned int ORDER = 12;
            static constexpr const unsigned int SIZE  = (1 << ORDER);
        };
    };

    struct Debug {
        static constexpr const bool ERROR = true;
        static constexpr const bool TRACE = true;
    };

    template <typename U>
    struct Scheduler;
};

template <>
struct Traits::Scheduler<Thread> {
    static constexpr unsigned long Frequency = Traits::Timer::MHz / 10;
    using Criterion                          = RR<Thread>;
};
