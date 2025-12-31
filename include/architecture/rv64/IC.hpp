#pragma once

template <typename Mode> class IC {

  public:
    using Context = RV64::Context<Mode>;

    enum {
        IS_INTERRUPT = 1ULL << (Traits<CPUS>::XLEN - 1),
    };

    static void error() {
        char p = Mode::PREFIX;
        auto status = reinterpret_cast<void *>(csrr<Mode::STATUS>());
        auto epc = reinterpret_cast<void *>(csrr<Mode::EPC>());
        auto tval = reinterpret_cast<void *>(csrr<Mode::TVAL>());
        auto cause = reinterpret_cast<void *>(csrr<Mode::CAUSE>());
        ERROR(true, "Ohh it's a Trap!\n%ccause: %d\n%cepc: %p\n%ctval: %p\n%cstatus: %p\n", p, cause, p, epc, p, tval,
              p, status);
    }
};

class MIC : IC<MachineMode> {
    static void handle(Context *context) {
        uintmax_t mcause = csrr<MachineMode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IS_INTERRUPT) {
            switch (code) {
            case Interrupt::TIMER:
                if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
                    csrc<MachineMode::IE>(MachineMode::TI);
                    csrs<MachineMode::IP>(SupervisorMode::TI);
                } else {
                    int core = CPU::id();
                    CLINT::reset(core);
                    Timer::handler(core);
                }
            }
        } else {
            if (mcause == Exception::SYSCALL) {
                context->pc += 4;
                Syscall::handle(reinterpret_cast<void *>(context->a0));
            } else {
                error();
            }
        }
    }

    enum Interrupt { TIMER = 7 };
    enum Exception { SYSCALL = 9 };

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handle(Context::push());
        Context::pop();
    }
};

class SIC : IC<SupervisorMode> {
    enum Interrupt { TIMER = 5 };

    static void handle(Context *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();
        int code = (scause << 1) >> 1;
        auto core = CPU::id();
        if (scause & IS_INTERRUPT) {
            switch (code) {
            case Interrupt::TIMER:
                CPU::syscall(CLINT::reset);
                Timer::handler(core);
                break;
            }
        } else {
            error();
        }
    }

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handle(Context::push());
        Context::pop();
    }
};

class Syscall {
    friend MIC;

  private:
    static void handle(void *function) {
        auto core = CPU::id();
        auto addr = reinterpret_cast<uintptr_t>(function);
        if (addr == reinterpret_cast<uintptr_t>(&CLINT::reset)) {
            CLINT::reset(core);
            csrs<MachineMode::IE>(MachineMode::TI);
            csrc<MachineMode::IP>(SupervisorMode::TI);
        } else {
            reinterpret_cast<void (*)()>(function)();
        }
    }
};
