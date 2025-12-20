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
        TIMER = 7,
    };

    enum Exception : unsigned long {
        SUPERVISOR_SYSCALL = 9,
    };

    enum Syscall {
        TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E',
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }

  public:
    class IC {
        static void handler(Context *c) {
            uintmax_t mcause = csrr<Machine::CAUSE>();

            if (mcause >> (Traits<::Machine>::XLEN - 1)) {
                Machine::Interruption code = static_cast<Machine::Interruption>((mcause << 1) >> 1);
                interruption(code);
                return;
            }

            if (mcause == Machine::SUPERVISOR_SYSCALL) {
                syscall(c);
                return;
            }

            error();
        }

        static void interruption(Machine::Interruption code) {
            switch (code) {
            case Machine::Interruption::TIMER:
                csrc<Machine::IE>(Machine::TI);
                csrs<Machine::IP>(Supervisor::TI);
            }
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
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Machine, true>());
            Context::pop<Machine, true>();
        }
    };

  public:
    static void init() {
        csrc<Machine::STATUS>(Machine::IRQE);

        if (!(csrr<Machine::MISA>() & (1UL << ('S' - 'A')))) {
            CPU::kill();
            CPU::idle();
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<Machine::IE>(Machine::TI);
        }

        csrw<Machine::SCRATCH>(s_stack[CPU::id()]);

        csrw<Machine::TVEC>(IC::entry);
        csrw<Machine::MIDELEG>(0x222);
        csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
        csrw<Machine::PMPCFG0>(0b11111);
        csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIRQE);
        csrc<Machine::STATUS>(Supervisor::PIRQE);
        csrw<Machine::EPC>(__builtin_return_address(0));
        Machine::ret();
    }

    static inline unsigned char s_stack[Traits<::Machine>::CPUS][Traits<Memory>::PAGE_SIZE];
};
