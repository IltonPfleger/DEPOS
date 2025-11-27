// class Machine {
//   public:
//     enum Registers : unsigned long {
//         MHARTID = 0xF14,  // Core ID
//         MEDELEG = 0x302,  // Machine Exception Delegation
//         MIDELEG = 0x303,  // Machine Interrupt Delegation
//         STATUS = 0x300,   // Machine Status
//         MISA = 0x301,     // ISA
//         IE = 0x304,       // Interrupt Enable
//         TVEC = 0x305,     // Trap Vector Base Address
//         SCRATCH = 0x340,  // Temporary Register
//         EPC = 0x341,      // Exception Program Counter
//         CAUSE = 0x342,    // Trap cause
//         TVAL = 0x343,     // Trap value
//         IP = 0x344,       // Pending Interrupts
//         PMPADDR0 = 0x3B0, // Physical Memory Protection Address
//         PMPCFG0 = 0x3A0   // Physical Memory Protection Config
//     };
//
//     enum Bits : unsigned long {
//         ME2ME = 3ULL << 11,         // Machine to Machine
//         ME2SUPERVISOR = 1ULL << 11, // Machine to Supervisor
//         ME2USER = 0ULL << 11,       // Machine to User
//         TI = 1ULL << 7,             // Timer Interrupt Enable
//         IRQE = 1ULL << 3,           // Interrupt Enable
//         PIRQE = 1ULL << 7           // Previous Interrupt Enable
//     };
//
//     __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }
// };

class Supervisor {
  public:
    enum Registers : unsigned long {
        STATUS = 0x100,  // Supervisor Status
        IE = 0x104,      // Interrupt Enable
        TVEC = 0x105,    // Trap Vector Base Address
        SCRATCH = 0x140, // Temporary Register
        EPC = 0x141,     // Exception Program Counter
        CAUSE = 0x142,   // Trap Cause
        TVAL = 0x143,    // Trap Value
        IP = 0x144,      // Pending Interrupts
        SATP = 0x180     // Page Table Base And MMU Mode
    };

    enum Bits : unsigned long {
        ME2ME = 1ULL << 8,   // Supervisor to Supervisor
        ME2USER = 0ULL << 8, // Supervisor to User
        IRQE = 1ULL << 1,    // Interrupt Enable
        TI = 1ULL << 5,      // Timer Interrupt Enable
        PIRQE = 1ULL << 5,   // Previous Interrupt Enable
        SUM = 1ULL << 18     // Supervisor User Memory access
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }
};
