#pragma once
#include <Machine.hpp>

template <typename T>
struct Traits;

template <typename T>
struct Scheduler;
template <typename T>
struct RR;
struct Thread;
template <>
struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = 1'000'000;
    using Criterion                          = RR<Thread>;
};

struct Timer;
template <>
struct Traits<Timer> {
    static constexpr const bool Enable             = true;
    static constexpr const unsigned long Frequency = 1'000'000;
};

struct Alarm;
template <>
struct Traits<Alarm> {
    static constexpr const bool Enable             = true;
    static constexpr const unsigned long Frequency = 1'000'000;
};

struct Memory;
template <>
struct Traits<Memory> {
    struct Page {
        static constexpr const unsigned int ORDER = 12;
        static constexpr const unsigned int SIZE  = (1 << ORDER);
    };
};

struct Debug;
template <>
struct Traits<Debug> {
    using Device                      = Machine::IO::UART;
    static constexpr const bool ERROR = true;
};
