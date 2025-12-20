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
        MACHINE2ME = 3ULL << 11,    // Machine to Machine
        ME2ME = MACHINE2ME,         // Machine to Machine
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

    enum Syscall {
        TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E',
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }

  public:
    class IC {
        static void handler(Context *c) {
            uintmax_t mcause = csrr<Machine::CAUSE>();

            if (mcause & IS_INTERRUPTION) {
                s_irqs.dispatch(mcause & ~IS_INTERRUPTION);
                return;
            }

            if (mcause == Machine::SYSCALL_FROM_SUPERVISOR) {
                syscall(c);
                return;
            }

            error();
        }

        static void reset_timer() {
            int core = CPU::id();
            CLINT::reset(core);
            Timer::handler(core);
        }

        static void forward_timer() {
            csrc<Machine::IE>(Machine::TI);
            csrs<Machine::IP>(Supervisor::TI);
        }

        static void syscall(Context *c) {
            c->pc += 4;
            switch (c->a7) {
            case Syscall::TIME:
                CLINT::reset(CPU::id());
                csrs<Machine::IE>(Machine::TI);
                csrc<Machine::IP>(Supervisor::TI);
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

      public:
        static void init() {
            if (CPU::id() == Traits<::Machine>::BSP) {
                if constexpr (Meta::SAME<KernelMode, Machine>::Result) {
                    s_irqs.bind(Interruption::TIMER, reset_timer);
                } else {
                    s_irqs.bind(Interruption::TIMER, forward_timer);
                }
            }

            CPU::barrier();
        }

        __attribute__((naked, aligned(4))) static void entry() {
            // handler(Context::push<Machine>());
            // Context::pop<Machine>();
            handler(Context::push<Machine, true>());
            Context::pop<Machine, true>();
        }

      private:
        static inline DispatchTable<Traits<IRQ>::MinMachineModeIRQ, Traits<IRQ>::MaxMachineModeIRQ> s_irqs;
    };

  public:
    __attribute__((noinline)) static void init() {
        csrc<STATUS>(IRQE);
        csrw<TVEC>(IC::entry);
        csrw<SCRATCH>(s_stack[CPU::id()] + Traits<Memory>::PAGE_SIZE);
        IC::init();

        if constexpr (!Meta::SAME<KernelMode, Machine>::Result) {
            if (!(csrr<Machine::MISA>() & (1UL << ('S' - 'A')))) {
                CPU::kill();
                CPU::idle();
            }

            csrw<Machine::MIDELEG>(0x222);
            csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<Machine::PMPCFG0>(0b11111);
            csrs<Machine::STATUS>(static_cast<unsigned long>(KernelMode::MACHINE2ME) | Machine::PIRQE);
            csrc<Machine::STATUS>(KernelMode::PIRQE);
            csrw<Machine::EPC>(__builtin_return_address(0));
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<Machine::IE>(Machine::TI);
        }

        if constexpr (!Meta::SAME<KernelMode, Machine>::Result) {
            Machine::ret();
        }
    }

    static inline unsigned char s_stack[Traits<::Machine>::CPUS][Traits<Memory>::PAGE_SIZE];
};
