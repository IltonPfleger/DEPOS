#pragma once

class Thread;
class Timer;
class RR;
class System;
class Application;
class Debug;
class Alarm;

class Machine;
class Memory;
class MemoryMap;
class Clock;

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<System> {
    static constexpr bool MULTITASK = true;
};

template <> struct Traits<Timer> {
    static constexpr bool Enable = true;
    static constexpr unsigned long MHz = 1'000'000;
    static constexpr unsigned long Frequency = MHz;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = Traits<Timer>::MHz / 10;
    using Criterion = RR;
};

#include <machine/rv64/SiFive_U/Traits.hpp>
// #include <machine/rv64/VisionFive2/Traits.hpp>
