#pragma once

#include <Meta.hpp>

class Thread;
class Machine;
class Timer;
class RR;
class System;
class Application;
class Debug;
class Alarm;
class Ethernet;
class Console;
class Dummy;
class Virtual;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<System> {
    static constexpr bool Multitask = false;
};

template <> struct Traits<Timer> {
    static constexpr unsigned long Frequency = 100'000;
    static constexpr bool Enable = true;
};

template <> struct Traits<Thread> {
    static constexpr unsigned long RescheduleFrequency = 10;
    static constexpr bool IsolatedKernelStack = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    using Criterion = RR;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 70;
};

template <> struct Traits<Dummy> {
    typedef Meta::TypeList<> Devices;
};

#include <application/Traits.hpp>
#include <machine/Traits.hpp>
