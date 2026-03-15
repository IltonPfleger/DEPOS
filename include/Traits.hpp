#pragma once

#include <Meta.hpp>
#include <headers.hpp>
#include <types.hpp>

namespace DEPOS {

class Thread;
class Machine;
class Timer;
class FixedCPU;
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
    static constexpr Hz Frequency = 10'000;
    static constexpr bool Enable  = true;
};

template <> struct Traits<Alarm> {
    static constexpr Hz Frequency = Traits<Timer>::Frequency;
    static constexpr bool Enable  = true;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 80;
};

} // namespace DEPOS

#include <application/Traits.hpp>
#include <machine/Traits.hpp>

namespace DEPOS {

template <> struct Traits<Thread> {
    static constexpr Hz RescheduleFrequency   = Traits<Timer>::Frequency;
    static constexpr bool IsolatedKernelStack = true;
    static constexpr unsigned UserStackSize   = Traits<Memory>::PageSize;
    static constexpr unsigned KernelStackSize = IsolatedKernelStack ? Traits<Memory>::PageSize : 0;
    using Criterion                           = RR;
    // using Criterion                           = FixedCPU;
};

} // namespace DEPOS
