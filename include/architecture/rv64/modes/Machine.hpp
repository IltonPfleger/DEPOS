class Machine {
  public:
    enum Registers : unsigned long {
        MHARTID = 0xF14,  // Core ID
        MEDELEG = 0x302,  // Machine Exception Delegation
        MIDELEG = 0x303,  // Machine Interrupt Delegation
        STATUS = 0x300,   // Machine Status
        MISA = 0x301,     // ISA
        IE = 0x304,       // Interrupt Enable
        TVEC = 0x305,     // Trap Vector Base Address
        SCRATCH = 0x340,  // Temporary Register
        EPC = 0x341,      // Exception Program Counter
        CAUSE = 0x342,    // Trap cause
        TVAL = 0x343,     // Trap value
        IP = 0x344,       // Pending Interrupts
        PMPADDR0 = 0x3B0, // Physical Memory Protection Address
        PMPCFG0 = 0x3A0   // Physical Memory Protection Config
    };

    enum Bits : unsigned long {
        ME2ME = 3ULL << 11,         // Machine to Machine
        ME2SUPERVISOR = 1ULL << 11, // Machine to Supervisor
        ME2USER = 0ULL << 11,       // Machine to User
        TI = 1ULL << 7,             // Timer Interrupt Enable
        IRQE = 1ULL << 3,           // Interrupt Enable
        PIRQE = 1ULL << 7           // Previous Interrupt Enable
    };

    enum InterruptionsCode : unsigned long {
        TIMER = 7,
    };

    enum ExceptionsCode : unsigned long {
        SYSCALL = 9,
    };

    class IC {

      public:
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Machine>());
            Context::pop<Machine>();
        }

        static void handler(Context *) {
            uintmax_t mcause = csrr<CAUSE>();
			bool is_interrupt = mcause >> (Traits<::Machine>::XLEN - 1);
            int code = (mcause << 1) >> 1;

            if (is_interrupt) {
                interrupt(code);
            } else {
                error();
            }
        }

        static void interrupt(int code) {
            switch (code) {
            case InterruptionsCode::TIMER:
                int core = CPU::id();
                CLINT::reset(core);
                Timer::handler(core);
            }
        }
        static void error() {
            auto mstatus = reinterpret_cast<void *>(csrr<STATUS>());
            auto mepc = reinterpret_cast<void *>(csrr<EPC>());
            auto mtval = reinterpret_cast<void *>(csrr<TVAL>());
            auto mcause = reinterpret_cast<void *>(csrr<CAUSE>());
            ERROR(true, "Ohh it's a Trap!", "\nmcause: ", mcause, "\nmepc: ", mepc, "\nmtval: ", mtval,
                  "\nmstatus: ", mstatus);
        }
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }
};
