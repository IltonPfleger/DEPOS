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
class Scheduler;
class Monitor;

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
    static constexpr bool Enable = true;
    static constexpr bool Error  = Enable && true;
    static constexpr bool Trace  = Enable && true;
};

template <> struct Traits<Console> {
    static constexpr size_t Columns = 100;
};

template <> struct Traits<Scheduler> {
    typedef FixedCore Criterion;
};

template <> struct Traits<Monitor> {
    static constexpr bool Enable   = true;
    static constexpr size_t Length = 100;
    static constexpr Hz Frequency  = Traits<Timer>::Frequency;
};

} // namespace DEPOS

#include __MACHINE_TRAITS_HEADER
#include __APPLICATION_TRAITS_HEADER

namespace DEPOS {

template <> struct Traits<Thread> {
    static constexpr Hz Frequency             = Traits<Timer>::Frequency;
    static constexpr bool IsolatedKernelStack = Traits<Application>::Virtualized || Traits<Debug>::Error;
    static constexpr size_t UserStackSize     = Traits<Memory>::StackSize;
    static constexpr size_t KernelStackSize   = IsolatedKernelStack ? Traits<Memory>::StackSize : 0;
};

} // namespace DEPOS

#endif
