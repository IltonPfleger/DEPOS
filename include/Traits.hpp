#pragma once

#include <Meta.hpp>

class Thread;
class Machine;
class Timer;
class RR;
class Kernel;
class Application;
class Debug;
class Alarm;
class Ethernet;
class Console;
class Virtual;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<Kernel> {
    static constexpr bool Multitask = false;
};

template <> struct Traits<Timer> {
    static constexpr unsigned long Frequency = 1'000;
    static constexpr bool Enable = true;
};

template <> struct Traits<Alarm> {
    static constexpr unsigned long Frequency = Traits<Timer>::Frequency;
    static constexpr bool Enable = true;
};

template <> struct Traits<Thread> {
    static constexpr unsigned long RescheduleFrequency = 1'000;
    static constexpr bool IsolatedKernelStack = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    using Criterion = RR;
};

template <> struct Traits<Debug> {
    static const bool error = true;
    static const bool warning = false;
    static const bool info = false;
    static const bool trace = false;
    static const bool logger = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 70;
};

#include <application/Traits.hpp>
#include <machine/Traits.hpp>
