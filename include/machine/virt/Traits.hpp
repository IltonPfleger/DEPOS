#pragma once

#include <Meta.hpp>

class Machine;
class MemoryMap;
class Memory;
class CPU;
class CLINT;
class PLIC;
class UART;
class RISCV;

template <typename T> struct Traits;

template <unsigned long, unsigned long, unsigned long> class UART16550;

template <> struct Traits<Machine> {
    static constexpr const char NAME[] = "virt";
};

template <> struct Traits<CPU> {
    static constexpr const char Architecture[] = "riscv64";
    static constexpr int Count = 10;
    static constexpr int Active = Count;
    static constexpr int BSP = 0;
};

template <> struct Traits<Memory> {
    static constexpr unsigned Order = 30;
    static constexpr unsigned Size = (1 << Order);
    static constexpr unsigned PageSize = 4096;
    static constexpr unsigned StackSize = PageSize;
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long PhysicalRamStart = 0x80000000;
    static constexpr unsigned long PhysicalRamEnd = PhysicalRamStart + Traits<Memory>::Size;

    static constexpr unsigned long VirtualRamStart = 0xffffffff80000000;
    static constexpr unsigned long VirtualRamEnd = VirtualRamStart + Traits<Memory>::Size;

    static constexpr unsigned long RamStart = Traits<System>::Multitask ? VirtualRamStart : PhysicalRamStart;
    static constexpr unsigned long RamEnd = Traits<System>::Multitask ? VirtualRamEnd : PhysicalRamEnd;

    static constexpr unsigned long SystemAddr = RamStart;

    static constexpr unsigned long MMIO = 0x00000000;
    static constexpr unsigned long UART0 = 0x10000000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
};

template <> struct Traits<UART> {
    typedef Meta::TypeList<UART16550<Traits<MemoryMap>::UART0, 115200, 10 + 11>> Devices;
    static constexpr unsigned int NumberOfDevices = Devices::Length;
};

template <> struct Traits<CLINT> {
    static constexpr bool Enable = Traits<Timer>::Enable;
    static constexpr unsigned long Addr = Traits<MemoryMap>::CLINT;
    static constexpr unsigned long Clock = 10'000'000;
};

template <> struct Traits<PLIC> {
    static constexpr bool Enable = false;
};
