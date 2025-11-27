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

    class Firmware {
      public:
        static void handler(Context *) {
            uintmax_t mcause = csrr<Machine::CAUSE>();
			bool is_interrupt = mcause >> (Traits<::Machine>::XLEN - 1);
            //int code = (mcause << 1) >> 1;

            if (is_interrupt) {
                csrc<Machine::IE>(Machine::TI);
                csrs<Machine::IP>(Supervisor::TI);
            } else {
                Machine::IC::error();
            }
        }

        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Machine>());
            Context::pop<Machine>();
        }
    };

    class IC {
        enum InterruptionsCode { TIMER = 5 };

        // static void error() {
        //     auto sepc = reinterpret_cast<void *>(csrr<Supervisor::EPC>());
        //     auto sstatus = reinterpret_cast<void *>(csrr<Supervisor::STATUS>());
        //     auto scause = reinterpret_cast<void *>(csrr<Supervisor::CAUSE>());
        //     auto stval = reinterpret_cast<void *>(csrr<Supervisor::TVAL>());
        //     ERROR(true, "Ohh it's a Trap!\nscause: %d\nsepc: %p\nstval: %p\nsstatus: %p\n", scause, sepc, stval,
        //           sstatus);
        // }

        static void handler(Context *) {
            uintmax_t scause = csrr<Supervisor::CAUSE>();
            bool is_interrupt = scause >> (Traits<::Machine>::XLEN - 1);
            int code = (scause << 1) >> 1;
            auto core = CPU::id();
            if (is_interrupt) {
                switch (code) {
                case InterruptionsCode::TIMER:
                    // CPU::syscall(CLINT::reset);
                    Timer::handler(core);
                    break;
                }
            }
        }

      public:
        __attribute__((naked, aligned(4))) static void entry() {
            handler(Context::push<Supervisor>());
            Context::pop<Supervisor>();
        }
    };

    __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }
};
