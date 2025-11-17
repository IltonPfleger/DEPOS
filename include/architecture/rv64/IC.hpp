#pragma once

class MIC {
    static void handler(void *args) {
        uintmax_t mcause = csrr<MachineMode::CAUSE>();
        bool is_interrupt = mcause >> (Traits<Machine>::XLEN - 1);
        int code = (mcause << 1) >> 1;

        if (is_interrupt) {
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
                Context *context = reinterpret_cast<Context *>(args);
                context->pc += 4;
                // Syscall::handler(reinterpret_cast<void *>(context->a0));
            } else {
                error();
            }
        }
    }

    static void error() {
        auto mstatus = reinterpret_cast<void *>(csrr<MachineMode::STATUS>());
        auto mepc = reinterpret_cast<void *>(csrr<MachineMode::EPC>());
        auto mtval = reinterpret_cast<void *>(csrr<MachineMode::TVAL>());
        Console::out << "ERROR: " << Console::Stream::endl;
        Console::out << "mstatus: " << mstatus << Console::Stream::endl;
        Console::out << "mepc: " << mepc << Console::Stream::endl;
        Console::out << "mtval: " << mtval << Console::Stream::endl;
        while (1)
            ;
    }

    enum Interrupt { TIMER = 7 };
    enum Exception { SYSCALL = 9 };

  public:
    __attribute__((naked, aligned(4))) static void entry() {
        handler(Context::push<MachineMode>());
        Context::pop<MachineMode>();
    }
};

class SIC {
    enum Interrupt { TIMER = 5 };

    static void error() {

        auto sstatus = reinterpret_cast<void *>(csrr<SupervisorMode::STATUS>());
        auto sepc = reinterpret_cast<void *>(csrr<SupervisorMode::EPC>());
        auto stval = reinterpret_cast<void *>(csrr<SupervisorMode::TVAL>());
        Console::out << "ERROR: " << Console::Stream::endl;
        Console::out << "sstatus: " << sstatus << Console::Stream::endl;
        Console::out << "sepc: " << sepc << Console::Stream::endl;
        Console::out << "stval: " << stval << Console::Stream::endl;
        while (1)
            ;
    }

    static void handler() {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();
        bool is_interrupt = scause >> (Traits<Machine>::XLEN - 1);
        int code = (scause << 1) >> 1;
        auto core = CPU::id();
        if (is_interrupt) {
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
        Context::push<SupervisorMode>();
        SIC::handler();
        Context::pop<SupervisorMode>();
    }
};

class Syscall {
    friend MIC;

  private:
    void handler(void *function) {
        auto core = CPU::id();
        auto addr = reinterpret_cast<uintptr_t>(function);
        if (addr == reinterpret_cast<uintptr_t>(&CLINT::reset)) {
            CLINT::reset(core);
            csrs<MachineMode::IE>(MachineMode::TI);
            csrc<MachineMode::IP>(SupervisorMode::TI);
        } else {
            // TODO: THIS EXECUTE IN MACHINE MODE
            reinterpret_cast<void (*)()>(function)();
            // ERROR(true, "Invalid Syscall!\n");
        }
    }
};
