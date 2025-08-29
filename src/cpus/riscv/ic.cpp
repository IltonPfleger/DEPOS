#include <IO/Debug.hpp>
#include <cpus/riscv/cpu.hpp>

void MIC::entry() {
    handler(RISCV::Context::push<RISCV::Machine>());
    RISCV::Context::pop<RISCV::Machine>();
}

void MIC::handler(void* args) {
    auto cause        = RISCV::csrr<RISCV::Machine::CAUSE>();
    bool is_interrupt = cause >> (Traits::Machine::XLEN - 1);
    auto code         = (cause << 1) >> 1;

    if (is_interrupt) {
        switch (code) {
            case 7:
                RISCV::csrc<RISCV::Machine::IE>(RISCV::Machine::TI);
                RISCV::csrs<RISCV::Machine::IP>(RISCV::Supervisor::TI);
                /// auto core = RISCV::core();
                /// RISCV::CLINT::reset(core);
                // RISCV::csrs<RISCV::Machine::IP>(RISCV::Supervisor::TI);
                // RISCV::csrs<RISCV::Supervisor::IP>(RISCV::Supervisor::TI);
                //    TRACE("%x\n", RISCV::csrr<RISCV::Machine::STATUS>());
                //      RISCV::csrc<RISCV::Machine::IE>(RISCV::Machine::TI);
                //        Timer::handler();
        }
    } else {
        RISCV::Context* context = reinterpret_cast<RISCV::Context*>(args);
        switch (cause) {
            case 9:
                context->pc += 4;
                Syscall::handler(static_cast<Syscall::Code>(context->a0));
                break;
            default:
                error();
                break;
        }
    }
}

void MIC::error() {
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    ERROR(true,
          "Ohh it's a Trap!\n"
          "mcause: %p\n"
          "mepc: %p\n"
          "mtval: %p\n",
          mcause, mepc, mtval);
}

void SIC::entry() {
    RISCV::Context::push<RISCV::Supervisor>();
    SIC::handler();
    RISCV::Context::pop<RISCV::Supervisor>();
}

void SIC::handler() {
    auto cause        = RISCV::csrr<RISCV::Supervisor::CAUSE>();
    bool is_interrupt = cause >> (Traits::Machine::XLEN - 1);
    auto code         = (cause << 1) >> 1;
    auto core         = RISCV::core();
    if (is_interrupt) {
        switch (code) {
            case 5:
                Syscall::call(Syscall::RESET_CLINT_TIMER);
                Timer::handler(core);
                break;
        }
    } else {
        error();
    }
}

void SIC::error() {
    uintptr_t scause, sepc, stval;
    __asm__ volatile("csrr %0, scause" : "=r"(scause));
    __asm__ volatile("csrr %0, sepc" : "=r"(sepc));
    __asm__ volatile("csrr %0, stval" : "=r"(stval));
    ERROR(true,
          "Ohh it's a Trap!\n"
          "scause: %p\n"
          "sepc: %p\n"
          "stval: %p\n",
          scause, sepc, stval);
}

void Syscall::call(Code code) {
    (void)code;
    RISCV::Machine::ecall();
}

void Syscall::handler(Code code) {
    auto core = RISCV::core();
    switch (code) {
        case RESET_CLINT_TIMER:
            RISCV::CLINT::reset(core);
            RISCV::csrs<RISCV::Machine::IE>(RISCV::Machine::TI);
            RISCV::csrc<RISCV::Machine::IP>(RISCV::Supervisor::TI);
    };
}
