class Machine;
class Memory;
class MemoryMap;
class Clock;
class IRQ;

template <> struct Traits<Machine> {
    static constexpr const char *NAME = "sifive_u";
    static constexpr int XLEN = 64;
    static constexpr int CPUS = 2;
    static constexpr int BSP = 1;
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
    static constexpr unsigned long PhysicalApplicationAddr = PhysicalRamStart + 1024 * 128;
    static constexpr unsigned long ApplicationAddr =
        Traits<System>::MULTITASK ? VirtualRamStart | PhysicalApplicationAddr : PhysicalApplicationAddr;

    /* *** MMIO Devices *** */
    static constexpr unsigned long UART = 0x10010000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
    static constexpr unsigned long MMIOStart = 0;
    static constexpr unsigned long MMIOEnd = 0x40000000;
};

template <> struct Traits<Clock> {
    static constexpr unsigned long CLINT = 1000000UL;
};

template <> struct Traits<IRQ> {
    static constexpr unsigned int MinMachineModeIRQ = 7;
    static constexpr unsigned int MaxMachineModeIRQ = 8;
    static constexpr unsigned int MinSupervisorModeIRQ = 5;
    static constexpr unsigned int MaxSupervisorModeIRQ = 6;
};

#define __MACHINE__ SiFive_U
