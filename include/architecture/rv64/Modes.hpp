struct MachineMode {
    enum : unsigned long {
        PMPADDR0 = 0x3B0,
        PMPCFG0 = 0x3A0,
        MHARTID = 0xF14,            // Core Number/ID
        MEDELEG = 0x302,            // Machine Exception Delegation
        MIDELEG = 0x303,            // Machine Interrupt Delegation
        ME2ME = 3ULL << 11,         // Machine to Machine
        ME2SUPERVISOR = 1ULL << 11, // Machine to Supervisor
        ME2USER = 0ULL << 11,       // Machine to User
        TI = 1ULL << 7,             // Timer Interrupt Enable
        IRQE = 1ULL << 3,           // Interrupt Enable
        PIRQE = 1ULL << 7,          // Previous Interrupt Enable
    };

    static constexpr const int STATUS = 0x300;
    static constexpr const int IE = 0x304;
    static constexpr const int TVEC = 0x305;
    static constexpr const int EPC = 0x341;
    static constexpr const int CAUSE = 0x342;
    static constexpr const int IP = 0x344;
    static constexpr const int TVAL = 0x343;

    __attribute__((always_inline)) static inline void ret() {
        asm volatile("mret");
    }
};

struct SupervisorMode {
    enum : unsigned long {
        ME2ME = 1ULL << 8,   // Supervisor to Supervisor
        ME2USER = 0ULL << 8, // Supervisor to User
        IRQE = 1ULL << 1,    // Interrupt Enable
        TI = 1ULL << 5,      // Timer Interrupt Enable
        PIRQE = 1ULL << 5,   // Previous Interrupt Enable
        SUM = 1ULL << 18,    // Supervisor User Memory
    };

    static constexpr const int SATP = 0x180;
    static constexpr const int STATUS = 0x100;
    static constexpr const int IE = 0x104;
    static constexpr const int TVEC = 0x105;
    static constexpr const int EPC = 0x141;
    static constexpr const int CAUSE = 0x142;
    static constexpr const int TVAL = 0x143;
    static constexpr const int IP = 0x144;
    __attribute__((always_inline)) static inline void ret() {
        asm volatile("sret");
    }
};
