#pragma once
#include <Machine.hpp>

struct Settings {
    struct IO {
        using Device = Machine::IO::UART;
    };
};

template <typename T>
struct Traits;

template <typename T>
struct Scheduler;
struct Thread;
template <typename T>
struct RR;
template <>
struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = 1'000;
    typedef RR<Thread> Criterion;
};

struct Timer;
template <>
struct Traits<Timer> {
    static constexpr const bool Enable             = true;
    static constexpr const unsigned long Frequency = 10'000;
};

struct Alarm;
template <>
struct Traits<Alarm> {
    static constexpr const bool Enable             = true;
    static constexpr const unsigned long Frequency = 1'000;
};

struct Debug;
template <>
struct Traits<Debug> {
    static constexpr const bool ERROR = true;
};
