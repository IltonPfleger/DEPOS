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
class CPU;

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

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    using Criterion = RR;
};

template <> struct Traits<Debug> {
    static const bool Error = true;
    static const bool Trace = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 70;
};

#include <application/Traits.hpp>
#include <machine/Traits.hpp>

template <> struct Traits<Thread> {
    static constexpr unsigned long RescheduleFrequency = 1'000;
    static constexpr unsigned int UserStackSize = Traits<Memory>::PageSize;
    static constexpr unsigned int KernelStackSize = Traits<Memory>::PageSize;
    static constexpr bool IsolatedKernelStack = true;
};
