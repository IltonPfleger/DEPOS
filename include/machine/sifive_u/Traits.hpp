#pragma once

template <typename T> struct Traits;
class MemoryMap;
class Memory;
class CPUS;
class Clock;

template <> struct Traits<CPUS> {
    static constexpr int XLEN = 64;
    static constexpr int COUNT = 2;
    static constexpr int ONLINE = 1;
    static constexpr int BSP = 0;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long Order = 30;
    static constexpr unsigned long Size = (1 << Order);
    static constexpr unsigned long PageOrder = 12;
    static constexpr unsigned long PageSize = (1 << PageOrder);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long RamStart = 0x80000000;
    static constexpr unsigned long RamEnd = RamStart + Traits<Memory>::Size;
    static constexpr unsigned long BootAddr = RamStart;

    static constexpr unsigned long ApplicationAddr = RamStart + 128 * 1024;

    static constexpr unsigned long UART = 0x10010000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
};

template <> struct Traits<Clock> {
    static constexpr unsigned long CLINT = 1'000'000;
};
