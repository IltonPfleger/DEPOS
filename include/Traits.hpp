#pragma once

class Thread;
class Machine;
class Timer;
class RR;
class System;
class Application;
class Debug;
class Alarm;
class Ethernet;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<System> {
    static constexpr bool Multitask = false;
};

template <> struct Traits<Timer> {
    static constexpr bool Enable = true;
    static constexpr unsigned long Frequency = 1'000;
};

template <> struct Traits<Alarm> {
    static constexpr bool Enable = true;
    static constexpr unsigned long Frequency = 10;

    static_assert(Frequency <= Traits<Timer>::Frequency);
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = Traits<Timer>::Frequency;
    static constexpr bool Preemptive = true;
    using Criterion = RR;

    static_assert(Frequency <= Traits<Timer>::Frequency);
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

#include <application/Traits.hpp>
#include <machine/Traits.hpp>
