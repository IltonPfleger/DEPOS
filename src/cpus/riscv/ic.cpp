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
        RISCV::Context* context = reinterpret_cast<RISCV::Context*>(args);
        switch (cause) {
            case Exception::SYSCALL:
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
    uintmax_t mcause = RISCV::csrr<RISCV::Machine::STATUS>();
    uintmax_t mepc   = RISCV::csrr<RISCV::Machine::EPC>();
    ERROR(true,
          "Ohh it's a Trap!\n"
          "mcause: %p\n"
          "mepc: %p\n",
          mcause, mepc);
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
            case Interrupt::TIMER:
                Syscall::call(Syscall::RESET_CLINT_TIMER);
                Timer::handler(core);
                break;
        }
    } else {
        error();
    }
}

void SIC::error() {
    uintmax_t scause = RISCV::csrr<RISCV::Supervisor::STATUS>();
    uintmax_t sepc   = RISCV::csrr<RISCV::Supervisor::EPC>();
    ERROR(true,
          "Ohh it's a Trap!\n"
          "scause: %p\n"
          "sepc: %p\n",
          scause, sepc);
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
