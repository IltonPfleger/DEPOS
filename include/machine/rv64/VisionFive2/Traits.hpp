template <> struct Traits<Machine> {
    static constexpr const char *NAME = "vision_five2";
    static constexpr int XLEN = 64;
    static constexpr int CPUS = 1;
    static constexpr int BSP = 0;
};

template <> struct Traits<Memory> {
    static constexpr unsigned long ORDER = 30;
    static constexpr unsigned long SIZE = (1 << ORDER);
    static constexpr unsigned long PAGE_ORDER = 12;
    static constexpr unsigned long PAGE_SIZE = (1 << PAGE_ORDER);
};

template <> struct Traits<MemoryMap> {
    static constexpr unsigned long PhysicalRamStart = 0x40000000;
    static constexpr unsigned long PhysicalRamEnd = PhysicalRamStart + Traits<Memory>::SIZE;
    static constexpr unsigned long PhysicalBootAddr = PhysicalRamStart;

    static constexpr unsigned long VirtualRamStart = 0xffffffff80000000;
    static constexpr unsigned long VirtualRamEnd = VirtualRamStart + Traits<Memory>::SIZE - 1;

    static constexpr unsigned long RamStart = Traits<System>::MULTITASK ? VirtualRamStart : PhysicalRamStart;
    static constexpr unsigned long RamEnd = Traits<System>::MULTITASK ? VirtualRamEnd : PhysicalRamEnd;

    static constexpr unsigned long SystemAddr = RamStart;
    static constexpr unsigned long PhysicalApplicationAddr = PhysicalRamStart + 1024 * 32;
    static constexpr unsigned long ApplicationAddr =
        Traits<System>::MULTITASK ? VirtualRamStart | PhysicalApplicationAddr : PhysicalApplicationAddr;

    /* *** MMIO Devices *** */
    static constexpr unsigned long UART = 0x10000000;
    static constexpr unsigned long CLINT = 0x02000000;
    static constexpr unsigned long PLIC = 0;
    static constexpr unsigned long MMIOStart = 0;
    static constexpr unsigned long MMIOEnd = 0x40000000;
};

template <> struct Traits<Clock> {
    static constexpr unsigned long CLINT = 10'000'000;
};

#define __MACHINE__ VisionFive2
