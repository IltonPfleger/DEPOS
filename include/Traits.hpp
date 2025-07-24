#pragma once
#include <Machine.hpp>

template <typename T>
struct Traits;

struct Timer;
template <>
struct Traits<Timer> {
    static constexpr const bool Enable             = true;
    static constexpr const unsigned long MHz       = 1'000'000;
    static constexpr const unsigned long Frequency = MHz;
};

template <typename T>
struct Scheduler;
template <typename T>
struct RR;
template <typename T>
struct RateMonotonic;
struct Thread;
template <>
struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = Traits<Timer>::MHz;
    using Criterion                          = RR<Thread>;
};

struct Alarm;
template <>
struct Traits<Alarm> {
    static constexpr const bool Enable             = false;
    static constexpr const unsigned long Frequency = Traits<Timer>::MHz;
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
    static constexpr const bool TRACE = true;
};
