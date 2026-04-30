#pragma once

#include <Meta.hpp>
#include <Traits.hpp>

namespace DEPOS {

class MemoryMap;
class Memory;
class Clock;
class CPU;
class CLINT;
class PLIC;
class UART;
class UART0;
class GMAC0;
class IC;
class CAN0;
class CacheController;
class Ethernet;

template <typename> class SiFiveU74CacheController;
template <typename> class UART16550;
template <typename> class DWC_Ether_QoS;
template <typename> class IPMS_CANFD;

template <> struct Traits<Machine> {
    static constexpr const char NAME[] = "VisionFive2";
};

template <> struct Traits<CPU> {
    static constexpr const char Architecture[] = "riscv64";
    static constexpr int Count                 = 5;
    static constexpr int Active                = Count - 1;
    static constexpr int Offset                = 1;
    static constexpr int BSP                   = 0;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long Order    = 30;
    static constexpr unsigned long Size     = (1 << Order);
    static constexpr unsigned int PageSize  = 4096;
    static constexpr unsigned int StackSize = PageSize;
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long PhysicalRamStart = 0x40000000;
    static constexpr unsigned long PhysicalRamEnd   = PhysicalRamStart + Traits<Memory>::Size;

    static constexpr unsigned long VirtualRamStart = 0xffffffff80000000;
    static constexpr unsigned long VirtualRamEnd   = VirtualRamStart + Traits<Memory>::Size;

    static constexpr unsigned long RamStart  = Traits<Kernel>::Multitask ? VirtualRamStart : PhysicalRamStart;
    static constexpr unsigned long RamEnd    = Traits<Kernel>::Multitask ? VirtualRamEnd : PhysicalRamEnd;
    static constexpr unsigned long BootStart = RamStart;

    static constexpr unsigned long KernelAddr = RamStart;

    /* *** MMIO *** */
    static constexpr unsigned long MMIO  = 0x00000000;
    static constexpr unsigned long GMAC0 = 0x16030000;
    static constexpr unsigned long GMAC1 = 0x16040000;
    static constexpr unsigned long UART0 = 0x10000000;
    static constexpr unsigned long CAN0  = 0x130D0000;
    static constexpr unsigned long CLINT = 0x2000000;
    static constexpr unsigned long PLIC  = 0x0C000000;
};

template <> struct Traits<CacheController> {
    typedef meta::TypeList<SiFiveU74CacheController<decltype(0)>> Devices;
    static constexpr unsigned int NumberOfDevices = Devices::Length;
};

template <> struct Traits<UART16550<UART0>> {
    static constexpr unsigned long Address = Traits<MemoryMap>::UART0;
    static constexpr unsigned int Clock    = 24'000'000;
    static constexpr unsigned int BaudRate = 115200;
    static constexpr unsigned int Shift    = 2;
    static constexpr unsigned int IRQs[]   = {32};
};

template <> struct Traits<UART> {
    typedef meta::TypeList<UART16550<UART0>> Devices;
    static constexpr unsigned int NumberOfDevices = Devices::Length;
};

template <> struct Traits<CAN0> {
    static constexpr unsigned long Address = Traits<MemoryMap>::CAN0;
    static constexpr unsigned int IRQs[]   = {112, 115};
};

template <> struct Traits<CLINT> {
    static constexpr bool Enable           = Traits<Timer>::Enable;
    static constexpr unsigned long Address = Traits<MemoryMap>::CLINT;
    static constexpr unsigned long Clock   = 4'000'000;
};

template <> struct Traits<DWC_Ether_QoS<GMAC0>> {
    static constexpr const char *MAC       = "12:34:56:78:12:34";
    static constexpr const char *IP        = "192.168.1.101";
    static constexpr unsigned long Address = Traits<MemoryMap>::GMAC0;
    static constexpr unsigned int IRQs[]   = {9};
};

template <> struct Traits<Ethernet> {
    typedef meta::TypeList<DWC_Ether_QoS<GMAC0>> Devices;
    static constexpr unsigned int NumberOfDevices = Devices::Length;
};

template <> struct Traits<PLIC> {
    static constexpr bool Enable               = true;
    static constexpr int NumberOfInterruptions = 127;
    static constexpr int Contexts[5][2]        = {
        {0, -1}, // Hart 0: M->0, S->N/A
        {1, 2},  // Hart 1: M->1, S->2
        {3, 4},  // Hart 2: M->3, S->4
        {5, 6},  // Hart 3: M->5, S->6
        {7, 8}   // Hart 4: M->7, S->8
    };
};

} // namespace DEPOS
