#ifndef __TRAITS_HEADER__
#define __TRAITS_HEADER__

#include <macros.hpp>
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
class Console;
class CPU;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<Kernel> {
    static constexpr bool Multitask = false;
};

template <> struct Traits<Timer> {
    static constexpr Hz TickFrequency = 10'000;
    static constexpr bool Enable      = true;
};

template <> struct Traits<Alarm> {
    static constexpr Hz TickFrequency = Traits<Timer>::TickFrequency;
    static constexpr bool Enable      = true;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Console> {
    static constexpr unsigned int Columns = 100;
};

} // namespace DEPOS

#include __MACHINE_TRAITS_HEADER
#include __APPLICATION_TRAITS_HEADER

namespace DEPOS {

template <> struct Traits<Thread> {
    static constexpr Hz TickFrequency         = Traits<Timer>::TickFrequency;
    static constexpr bool IsolatedKernelStack = Traits<Application>::Virtualized || Traits<Debug>::Error;
    static constexpr size_t UserStackSize     = Traits<Memory>::PageSize;
    static constexpr size_t KernelStackSize   = IsolatedKernelStack ? Traits<Memory>::PageSize : 0;
    using Criterion                           = RR;
};

} // namespace DEPOS

#endif
