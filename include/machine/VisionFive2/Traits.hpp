#pragma once

#include <Traits.hpp>

class MemoryMap;
class Memory;
class Clock;
class CPUS;
class CLINT;
class PLIC;
class GMAC0;

template <> struct Traits<CPUS> {
    static constexpr int XLEN = 64;
    static constexpr int COUNT = 1;
    static constexpr int ONLINE = COUNT;
    static constexpr int BSP = 0;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long Order = 30;
    static constexpr unsigned long Size = (1 << Order);
    static constexpr unsigned long PageOrder = 12;
    static constexpr unsigned long PageSize = (1 << PageOrder);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long RamStart = 0x40000000;
    static constexpr unsigned long RamEnd = RamStart + Traits<Memory>::Size;
    static constexpr unsigned long BootAddr = RamStart;

    static constexpr unsigned long ApplicationAddr = RamStart + 128 * 1024;

    /* *** MMIO *** */
    static constexpr unsigned long CacheController = 0x2010000;
    static constexpr unsigned long GMAC0 = 0x16030000;
    static constexpr unsigned long GMAC1 = 0x16040000;
    static constexpr unsigned long UART = 0x10000000;
    static constexpr unsigned long CLINT = 0x2000000;
    static constexpr unsigned long PLIC = 0x0C000000;
};

template <> struct Traits<GMAC0> {
    static constexpr unsigned int IRQs[] = {19};
};

template <> struct Traits<CLINT> {
    static constexpr bool Enable = Traits<Timer>::Enable;
    static constexpr unsigned long Addr = Traits<MemoryMap>::CLINT;
    static constexpr unsigned long Clock = 4'000'000;
};

template <> struct Traits<PLIC> {
    static constexpr bool Enable = true;
    static constexpr unsigned long Addr = Traits<MemoryMap>::PLIC;
    static constexpr unsigned int First = 0;
    static constexpr unsigned int Last = 136;
    static constexpr unsigned int Count = Last - First + 1;
};
