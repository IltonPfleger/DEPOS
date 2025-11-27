#pragma once

// class MIC {
//     static void handler(void *args) {
//         uintmax_t mcause = csrr<Machine::CAUSE>();
//         bool is_interrupt = mcause >> (Traits<::Machine>::XLEN - 1);
//         int code = (mcause << 1) >> 1;
//
//         if (is_interrupt) {
//             switch (code) {
//             case Interrupt::TIMER:
//                 if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
//                     csrc<Machine::IE>(Machine::TI);
//                     csrs<Machine::IP>(Supervisor::TI);
//                 } else {
//                     int core = CPU::id();
//                     CLINT::reset(core);
//                     Timer::handler(core);
//                 }
//             }
//         } else {
//             if (mcause == Exception::SYSCALL) {
//                 Context *context = reinterpret_cast<Context *>(args);
//                 context->pc += 4;
//                 Syscall::handler(reinterpret_cast<void *>(context->a0));
//             } else {
//                 error();
//             }
//         }
//     }
//
//     static void error() {
//         auto mstatus = reinterpret_cast<void *>(csrr<Machine::STATUS>());
//         auto mepc = reinterpret_cast<void *>(csrr<Machine::EPC>());
//         auto mtval = reinterpret_cast<void *>(csrr<Machine::TVAL>());
//         auto mcause = reinterpret_cast<void *>(csrr<Machine::CAUSE>());
//         ERROR(true, "Ohh it's a Trap!\nmcause: %d\nmepc: %p\nmtval: %p\nmstatus: %p\n", mcause, mepc, mtval,
//         mstatus);
//     }
//
//     enum Interrupt { TIMER = 7 };
//     enum Exception { SYSCALL = 9 };
//
//   public:
//     __attribute__((naked, aligned(4))) static void entry() {
//         handler(Context::push<Machine>());
//         Context::pop<Machine>();
//     }
// };

class SIC {
    enum Interrupt { TIMER = 5 };

    static void error() {
        auto sepc = reinterpret_cast<void *>(csrr<Supervisor::EPC>());
        auto sstatus = reinterpret_cast<void *>(csrr<Supervisor::STATUS>());
        auto scause = reinterpret_cast<void *>(csrr<Supervisor::CAUSE>());
        auto stval = reinterpret_cast<void *>(csrr<Supervisor::TVAL>());
        ERROR(true, "Ohh it's a Trap!\nscause: %d\nsepc: %p\nstval: %p\nsstatus: %p\n", scause, sepc, stval, sstatus);
    }

    static void handler() {
        uintmax_t scause = csrr<Supervisor::CAUSE>();
        bool is_interrupt = scause >> (Traits<::Machine>::XLEN - 1);
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
        Context::push<Supervisor>();
        SIC::handler();
        Context::pop<Supervisor>();
    }
};

class Syscall {

  private:
    static void handler(void *function) {
        auto core = CPU::id();
        auto addr = reinterpret_cast<uintptr_t>(function);
        if (addr == reinterpret_cast<uintptr_t>(&CLINT::reset)) {
            CLINT::reset(core);
            csrs<Machine::IE>(Machine::TI);
            csrc<Machine::IP>(Supervisor::TI);
        }
    }
};
