#pragma once

#include <Meta.hpp>
#include <Traits.hpp>

class MemoryMap;
class Memory;
class CPUS;
class CLINT;
class PLIC;
class UART;

template <unsigned long> class SiFiveUART;

template <> struct Traits<CPUS> {
    static constexpr int XLEN = 64;
    static constexpr int COUNT = 2;
    static constexpr int ACTIVE = Traits<System>::Multitask ? COUNT - 1 : COUNT;
    static constexpr int BSP = 1;
};

template <> struct Traits<Memory> {
    static constexpr unsigned Order = 30;
    static constexpr unsigned Size = (1 << Order);
    static constexpr unsigned PageOrder = 12;
    static constexpr unsigned PageSize = (1 << PageOrder);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long PhysicalRamStart = 0x80000000;
    static constexpr unsigned long PhysicalRamEnd = PhysicalRamStart + Traits<Memory>::Size;

    static constexpr unsigned long VirtualRamStart = 0xffffffff80000000;
    static constexpr unsigned long VirtualRamEnd = VirtualRamStart + Traits<Memory>::Size;

    static constexpr unsigned long RamStart = Traits<System>::Multitask ? VirtualRamStart : PhysicalRamStart;
    static constexpr unsigned long RamEnd = Traits<System>::Multitask ? VirtualRamEnd : PhysicalRamEnd;

    static constexpr unsigned long SystemAddr = RamStart;
    static constexpr unsigned long ApplicationAddr = RamStart + 128 * 1024;

    static constexpr unsigned long UART0 = 0x10010000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
};

template <> struct Traits<UART> {
    typedef Meta::TypeList<SiFiveUART<Traits<MemoryMap>::UART0>> Devices;
    static constexpr unsigned int NumberOfDevices = Devices::Length;
};

template <> struct Traits<CLINT> {
    static constexpr bool Enable = Traits<Timer>::Enable;
    static constexpr unsigned long Addr = Traits<MemoryMap>::CLINT;
    static constexpr unsigned long Clock = 1'000'000;
};

template <> struct Traits<PLIC> {
    static constexpr bool Enable = false;
};
