#pragma once

class Machine {
    using Stack = unsigned char[Traits<Memory>::PAGE_SIZE];
    static constexpr bool Main = Meta::SAME<KernelMode, Machine>::Result;

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
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Machine, !Main>());
            Context::pop<Machine, !Main>();
        }

        static void handler(Context *c) {
            unsigned long mcause = csrr<Machine::CAUSE>();

            if (mcause & IS_INTERRUPTION) {
                s_irqs.dispatch(mcause & ~IS_INTERRUPTION);
                return;
            }

            if constexpr (!Meta::SAME<KernelMode, Machine>::Result) {
                if (mcause == Machine::SYSCALL_FROM_SUPERVISOR) {
                    syscall(c);
                    return;
                }
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
            csrw<TVEC>(entry);
            if (CPU::id() != Traits<::Machine>::BSP) {
                CPU::barrier();
                return;
            }

            if constexpr (Traits<Timer>::Enable) {
                csrs<IE>(TI);
                if constexpr (Main) {
                    s_irqs.bind(Interruption::TIMER, reset_timer);
                } else {
                    s_irqs.bind(Interruption::TIMER, forward_timer);
                }
            }

            CPU::barrier();
        }

      private:
        static inline DispatchTable<Traits<IRQ>::MinMachineModeIRQ, Traits<IRQ>::MaxMachineModeIRQ> s_irqs;
    };

  public:
    __attribute__((noinline)) static void init() {
        csrc<STATUS>(IRQE);

        IC::init();

        if constexpr (!Main) {
            Stack *stack = reinterpret_cast<Stack *>(&s_stack[CPU::id()].Result);
            csrw<SCRATCH>(*stack + Traits<Memory>::PAGE_SIZE);
            if (!(csrr<MISA>() & (1UL << ('S' - 'A')))) {
                CPU::kill();
                CPU::idle();
            }

            csrw<MIDELEG>(0x222);
            csrw<PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<PMPCFG0>(0b11111);
            csrs<STATUS>(static_cast<unsigned long>(KernelMode::MACHINE2ME) | PIRQE);
            csrc<STATUS>(KernelMode::PIRQE);
            csrw<EPC>(__builtin_return_address(0));
        }

        if constexpr (!Main) {
            ret();
        }
    }

    static inline Meta::ConditionalValue<Stack, !Main> s_stack[Traits<::Machine>::CPUS];
};
