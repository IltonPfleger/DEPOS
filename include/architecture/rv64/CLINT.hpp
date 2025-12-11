class CLINT {
    static constexpr const uintptr_t Addr = Traits<MemoryMap>::CLINT;
    static volatile inline uintmax_t *MTIMECMP = reinterpret_cast<volatile uintmax_t *>(Addr + 0x4000);
    static volatile inline uintmax_t *MTIME = reinterpret_cast<volatile uintmax_t *>(Addr + 0xBFF8);

  public:
    static void reset(unsigned long core) {
        static constexpr uintmax_t ticks = Traits<Clock>::CLINT / Traits<Timer>::Frequency;
        MTIMECMP[core] = *MTIME + ticks;
    }
};
