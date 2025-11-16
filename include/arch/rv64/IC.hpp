class MIC {
    static void handler(void *args) {
        uintmax_t mcause = csrr<Machine::CAUSE>();
        bool is_interrupt = mcause >> (Traits<::Machine>::XLEN - 1);
        int code = (mcause << 1) >> 1;

        if (is_interrupt) {
            switch (code) {
            case Interrupt::TIMER:
                if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
                    csrc<Machine::IE>(Machine::TI);
                    csrs<Machine::IP>(Supervisor::TI);
                } else {
                    int _core = core();
                    CLINT::reset(_core);
                    Timer::handler(_core);
                }
            }
        } else {
            if (mcause == Exception::SYSCALL) {
                Context *context = reinterpret_cast<Context *>(args);
                context->pc += 4;
                // Syscall::handler(reinterpret_cast<void *>(context->a0));
            } else {
                error();
            }
        }
    }

    static void error() {
        // auto mstatus = reinterpret_cast<void *>(csrr<Machine::STATUS>());
        // auto mepc = reinterpret_cast<void *>(csrr<Machine::EPC>());
        // auto mtval = reinterpret_cast<void *>(csrr<Machine::TVAL>());
        //  ERROR(
        //      true,
        //      "Ohh it's a Trap!\nmcause: %d\nmepc: %p\nmtval: %p\nmstatus:
        //      %p\n", mcause, mepc, mtval, mstatus);
    }

    enum Interrupt { TIMER = 7 };
    enum Exception { SYSCALL = 9 };

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handler(Context::push<Machine>());
        Context::pop<Machine>();
    }
};

class SIC {
    enum Interrupt { TIMER = 5 };

    static void error() {
        // auto sepc = reinterpret_cast<void *>(csrr<Supervisor::EPC>());
        //  ERROR(true,
        //        "Ohh it's a Trap!\n"
        //        "scause: ",
        //        scause, "\nsepc: ", sepc, "\n");
    }

    static void handler() {
        uintmax_t scause = csrr<Supervisor::CAUSE>();
        bool is_interrupt = scause >> (Traits<::Machine>::XLEN - 1);
        int code = (scause << 1) >> 1;
        auto _core = core();
        if (is_interrupt) {
            switch (code) {
            case Interrupt::TIMER:
                syscall(CLINT::reset);
                Timer::handler(_core);
                break;
            }
        } else {
            error();
        }
    }

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        Context::push<Supervisor>();
        SIC::handler();
        Context::pop<Supervisor>();
    }
};

class Syscall {
    friend MIC;

  private:
    void handler(void *function) {
        auto _core = core();
        auto addr = reinterpret_cast<uintptr_t>(function);
        if (addr == reinterpret_cast<uintptr_t>(&CLINT::reset)) {
            CLINT::reset(_core);
            csrs<Machine::IE>(Machine::TI);
            csrc<Machine::IP>(Supervisor::TI);
        } else {
            // TODO: THIS EXECUTE IN MACHINE MODE
            reinterpret_cast<void (*)()>(function)();
            // ERROR(true, "Invalid Syscall!\n");
        }
    }
};
