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

template <typename U> class Scheduler;

template <typename T> struct Traits;

template <> struct Traits<Machine> {
    static constexpr const char *NAME = "sifive_u";
    static constexpr int XLEN = 64;
    static constexpr int CPUS = 5;
    static constexpr int BSP = 1;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long ORDER = 30;
    static constexpr unsigned long SIZE = (1 << ORDER);
    static constexpr unsigned long PAGE_ORDER = 12;
    static constexpr unsigned long PAGE_SIZE = (1 << PAGE_ORDER);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long RAM_BASE = 0x80000000;
    static constexpr unsigned long RAM_END = RAM_BASE + Traits<Memory>::SIZE;
    static constexpr unsigned long BOOT_ADDR = RAM_BASE;

    /* *** Devices *** */
    static constexpr unsigned long UART = 0x10010000ULL;
    static constexpr unsigned long CLINT = 0x02000000ULL;
    static constexpr unsigned long PLIC = 0;
};

template <> struct Traits<Timer> {
    static constexpr bool Enable = true;
    static constexpr unsigned long MHz = 1'000'000;
    static constexpr unsigned long Frequency = MHz;
};

template <> struct Traits<Alarm> {
    static constexpr bool Enable = true;
    static constexpr unsigned long Frequency = Traits<Timer>::MHz;
};

template <> struct Traits<System> {
    static constexpr bool MULTITASK = false;
};

template <> struct Traits<Application> {
    static constexpr unsigned long ADDR = 0xA0000000;
};

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = true;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = Traits<Timer>::MHz / 10;
    static constexpr bool Preemptive = true;
    using Criterion = RR;
};
