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
    static constexpr unsigned long Frequency = 100;
};

template <> struct Traits<Alarm> {
    static constexpr bool Enable = true;
    static constexpr unsigned long Frequency = 10;

    static_assert(Traits<Timer>::Enable || !Enable);
    static_assert(Frequency <= Traits<Timer>::Frequency);
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    static constexpr unsigned long Frequency = 10;
    using Criterion = RR;

    static_assert(Traits<Timer>::Enable || !Preemptive);
    static_assert(Frequency <= Traits<Timer>::Frequency);
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

#include <machine/Traits.hpp>
/**/
#include <application/Traits.hpp>
