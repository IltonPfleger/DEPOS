#pragma once

class Thread;
class Machine;
class Timer;
class RR;
class Memory;
class System;
class Application;
class Debug;
class Alarm;
class MemoryMap;
class CPUS;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<Machine> {
    static constexpr const char *NAME = "sifive_u";
};

template <> struct Traits<System> {
    static constexpr bool MULTITASK = false;
};

template <> struct Traits<Timer> {
    static constexpr bool Enable = true;
    // static constexpr unsigned long MHz = 1'000'000;
    static constexpr unsigned long Frequency = 10'000;
};

// template <> struct Traits<Alarm> {
//     static constexpr bool Enable = true;
//     static constexpr unsigned long Frequency = Traits<Timer>::MHz;
// };

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    static constexpr unsigned long Frequency = 1000;
    using Criterion = RR;

    static_assert(Frequency <= Traits<Timer>::Frequency);
};
