#pragma once

#include <Meta.hpp>

namespace DEPOS {

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
    static constexpr unsigned long Frequency = 1'000'000;
    static constexpr bool Enable             = true;
};

template <> struct Traits<Alarm> {
    static constexpr unsigned long Frequency = Traits<Timer>::Frequency;
    static constexpr bool Enable             = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr bool Preemptive = true;
    using Criterion                  = RR;
};

template <> struct Traits<Debug> {
    static const bool Error = true;
    static const bool Trace = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 70;
};

} // namespace DEPOS

#include <application/Traits.hpp>
#include <machine/Traits.hpp>

namespace DEPOS {

template <> struct Traits<Thread> {
    static constexpr unsigned long RescheduleFrequency = Traits<Timer>::Frequency;
    static constexpr bool IsolatedKernelStack          = true;
    static constexpr unsigned UserStackSize            = Traits<Memory>::PageSize;
    static constexpr unsigned KernelStackSize = IsolatedKernelStack ? Traits<Memory>::PageSize : 0;
};

} // namespace DEPOS
