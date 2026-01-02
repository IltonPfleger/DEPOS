#pragma once

template <typename Mode> class IC {

  public:
    using Context = ContextBase<Mode>;

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

class MIC : public IC<MachineMode> {
    using Mode = MachineMode;
    using Base = IC<Mode>;
    using Context = typename Base::Context;

    static void handle(Context *context) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & Base::IS_INTERRUPT) {
            switch (code) {
            case TIMER:
                if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
                    csrc<Mode::IE>(Mode::TI);
                    csrs<Mode::IP>(SupervisorMode::TI);
                } else {
                    int core = CPU::id();
                    TimerDevice::reset(core);
                    Timer::handler(core);
                }
            }
            return;
        }
        if (mcause == SYSCALL_FROM_SUPERVISOR) {
            context->pc += 4;
            Syscall::handle(reinterpret_cast<void *>(context->a0));
            return;
        }

        Base::error();
    }

    enum { TIMER = 7, SYSCALL_FROM_SUPERVISOR = 9 };

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handle(Context::push());
        Context::pop();
    }
};

class SIC : IC<SupervisorMode> {
    using Base = IC<SupervisorMode>;
    using Context = typename Base::Context;

    enum Interrupt { TIMER = 5 };

    static void handle(Context *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();
        int code = (scause << 1) >> 1;
        if (scause & Base::IS_INTERRUPT) {
            switch (code) {
            case Interrupt::TIMER:
                auto core = CPU::id();
                CPU::syscall(TimerDevice::reset);
                Timer::handler(core);
                break;
            }
            return;
        }

        Base::error();
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
        if (addr == reinterpret_cast<uintptr_t>(&TimerDevice::reset)) {
            TimerDevice::reset(core);
            csrs<MachineMode::IE>(MachineMode::TI);
            csrc<MachineMode::IP>(SupervisorMode::TI);
        } else {
            reinterpret_cast<void (*)()>(function)();
        }
    }
};
