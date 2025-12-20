#pragma once

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

    enum Interruption : unsigned long {
        IS_INTERRUPTION = 1ULL << (Traits<::Machine>::XLEN - 1),
        TIMER = 7,
    };

    enum Exception : unsigned long {
        SYSCALL_FROM_SUPERVISOR = 9,
    };

    class IC {

      public:
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Machine>());
            Context::pop<Machine>();
        }

        static void handler(Context *) {
            uintmax_t mcause = csrr<CAUSE>();

            if (mcause & IS_INTERRUPTION) {
                int code = mcause & ~IS_INTERRUPTION;
                s_irqs.dispatch(code);
                return;
            }

            error();
        }

        static void clint() {
            int core = CPU::id();
            CLINT::reset(core);
            Timer::handler(core);
        }

        static void error() {
            auto mepc = reinterpret_cast<void *>(csrr<EPC>());
            auto mtval = reinterpret_cast<void *>(csrr<TVAL>());
            auto mcause = reinterpret_cast<void *>(csrr<CAUSE>());
            ERROR(true, "Ohh it's a Trap!", "\nmcause: ", mcause, "\nmepc: ", mepc, "\nmtval: ", mtval);
        }

        static void init() { s_irqs.bind(Interruption::TIMER, clint); }

      private:
        static inline DispatchTable<Traits<IRQ>::MinMachineModeIRQ, Traits<IRQ>::MaxMachineModeIRQ> s_irqs;
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }

    static void init() {
        csrc<Machine::STATUS>(Machine::IRQE);

        if (CPU::id() == Traits<::Machine>::BSP) {
            IC::init();
        }

        CPU::barrier();

        if constexpr (Traits<Timer>::Enable) {
            csrs<Machine::IE>(Machine::TI);
        }

        csrw<Machine::TVEC>(Machine::IC::entry);
    }
};
