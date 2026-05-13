#ifndef __DEPOS_TRAITS__
#define __DEPOS_TRAITS__

#include <macros.hpp>
#include <types.hpp>

namespace DEPOS {

class Thread;
class Machine;
class Timer;
class FixedCore;
class RR;
class Kernel;
class Application;
class Debug;
class Alarm;
class Console;
class CPU;
class IPv4;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<Kernel> {
    static constexpr bool Multitask = false;
};

template <> struct Traits<Timer> {
    static constexpr Hz TickFrequency = 100'000;
    static constexpr bool Enable      = true;
};

template <> struct Traits<Alarm> {
    static constexpr Hz TickFrequency = Traits<Timer>::TickFrequency;
    static constexpr bool Enable      = true;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = false;
    static constexpr bool Trace = false;
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
    static constexpr size_t UserStackSize     = Traits<Memory>::StackSize;
    static constexpr size_t KernelStackSize   = IsolatedKernelStack ? Traits<Memory>::StackSize : 0;
    using Criterion                           = FixedCore;
};

} // namespace DEPOS

#endif
