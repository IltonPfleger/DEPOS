#include <IO/Debug.hpp>
#include <cpus/riscv/cpu.hpp>

void MIC::entry() {
    handler(RISCV::Context::push<RISCV::Machine>());
    RISCV::Context::pop<RISCV::Machine>();
}

void MIC::handler(void* args) {
    uintmax_t mcause  = RISCV::csrr<RISCV::Machine::CAUSE>();
    bool is_interrupt = mcause >> (Traits::Machine::XLEN - 1);
    int code          = (mcause << 1) >> 1;

    if (is_interrupt) {
        switch (code) {
            case Interrupt::TIMER:
                if constexpr (Meta::SAME<RISCV::Mode, RISCV::Supervisor>::Result) {
                    RISCV::csrc<RISCV::Machine::IE>(RISCV::Machine::TI);
                    RISCV::csrs<RISCV::Machine::IP>(RISCV::Supervisor::TI);
                } else {
                    auto core = RISCV::core();
                    RISCV::CLINT::reset(core);
                    Timer::handler(core);
                }
        }
    } else {
        if (mcause == Exception::SYSCALL) {
            RISCV::Context* context = reinterpret_cast<RISCV::Context*>(args);
            context->pc += 4;
            Syscall::handler(static_cast<Syscall::Code>(context->a0));
        } else {
            error(mcause);
        }
    }
}

void MIC::error(intmax_t mcause) {
    auto mepc  = reinterpret_cast<void*>(RISCV::csrr<RISCV::Machine::EPC>());
    auto mtval = reinterpret_cast<void*>(RISCV::csrr<RISCV::Machine::TVAL>());
    ERROR(true,
          "Ohh it's a Trap!\n"
          "mcause: ",
          mcause, "\nmepc: ", mepc, "\nmtval: ", mtval, "\n");
}

void SIC::entry() {
    RISCV::Context::push<RISCV::Supervisor>();
    SIC::handler();
    RISCV::Context::pop<RISCV::Supervisor>();
}

void SIC::handler() {
    uintmax_t scause  = RISCV::csrr<RISCV::Supervisor::CAUSE>();
    bool is_interrupt = scause >> (Traits::Machine::XLEN - 1);
    int code          = (scause << 1) >> 1;
    auto core         = RISCV::core();
    if (is_interrupt) {
        switch (code) {
            case Interrupt::TIMER:
                Syscall::call(Syscall::RESET_CLINT_TIMER);
                Timer::handler(core);
                break;
        }
    } else {
        error(scause);
    }
}

void SIC::error(intmax_t scause) {
    auto sepc = reinterpret_cast<void*>(RISCV::csrr<RISCV::Supervisor::EPC>());
    ERROR(true,
          "Ohh it's a Trap!\n"
          "scause: ",
          scause, "\nsepc: ", sepc, "\n");
}

void Syscall::call(Code code) {
    asm volatile("mv a0, %0" ::"r"(code) : "memory");
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
