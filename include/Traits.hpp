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
    static constexpr int CPUS = 2;
    static constexpr int BSP = 1;
};

template <> struct Traits<System> {
    static constexpr bool MULTITASK = false;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long ORDER = 30;
    static constexpr unsigned long SIZE = (1 << ORDER);
    static constexpr unsigned long PAGE_ORDER = 12;
    static constexpr unsigned long PAGE_SIZE = (1 << PAGE_ORDER);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long PhysicalRamStart = 0x80000000;
    static constexpr unsigned long PhysicalRamEnd = PhysicalRamStart + Traits<Memory>::SIZE - 1;
    static constexpr unsigned long PhysicalBootAddr = PhysicalRamStart;

    static constexpr unsigned long VirtualRamStart = 0xffffffff80000000;
    static constexpr unsigned long VirtualRamEnd = VirtualRamStart + Traits<Memory>::SIZE - 1;

    static constexpr unsigned long RamStart = Traits<System>::MULTITASK ? VirtualRamStart : PhysicalRamStart;
    static constexpr unsigned long RamEnd = Traits<System>::MULTITASK ? VirtualRamEnd : PhysicalRamEnd;

    static constexpr unsigned long SystemAddr = RamStart;
    static constexpr unsigned long ApplicationAddr = SystemAddr + Traits<Memory>::SIZE / 2;

    /* *** MMIO Devices *** */
    static constexpr unsigned long UART = 0x10010000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
    static constexpr unsigned long MMIOStart = 0;
    static constexpr unsigned long MMIOEnd = 0x40000000;
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

template <> struct Traits<Debug> {
    static constexpr bool Error = true;
    static constexpr bool Trace = false;
};

template <> struct Traits<Scheduler<Thread>> {
    static constexpr unsigned long Frequency = Traits<Timer>::MHz / 10;
    using Criterion = RR;
};
