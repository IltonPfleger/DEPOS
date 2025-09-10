#pragma once

struct Thread;
template <typename T>
struct RR;
template <typename T>
struct RateMonotonic;

struct Traits {
    struct Machine {
        static constexpr const char *NAME = "sifive_u";
        static constexpr int XLEN         = 64;
        static constexpr int CPUS         = 5;
        static constexpr int BSP          = 1;
    };

    struct System {
        static constexpr unsigned long RAM_BASE  = 0x80000000;
        static constexpr unsigned long BASE_PHYS = 0x80000000;
        static constexpr unsigned long BASE_VIRT = 0xFFFFFFFFC0000000;
        static constexpr int MULTITASK           = true;
    };

    struct Timer {
        static constexpr bool Enable             = true;
        static constexpr unsigned long MHz       = 1'000'000;
        static constexpr unsigned long Frequency = MHz;
    };

    struct Alarm {
        static constexpr bool Enable             = true;
        static constexpr unsigned long Frequency = Timer::MHz;
    };

    struct Memory {
        static constexpr unsigned int ORDER = 30;
        static constexpr unsigned int SIZE  = (1 << ORDER);
        struct Page {
            static constexpr unsigned int ORDER = 12;
            static constexpr unsigned int SIZE  = (1 << ORDER);
        };
    };

    struct Debug {
        static constexpr bool ERROR = true;
        static constexpr bool TRACE = true;
    };

    template <typename U>
    struct Scheduler;
};

template <>
struct Traits::Scheduler<Thread> {
    static constexpr unsigned long Frequency = Traits::Timer::MHz / 10;
    using Criterion                          = RR<Thread>;
};
