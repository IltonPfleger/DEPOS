#include <IO/Debug.hpp>
#include <Timer.hpp>
#include <arch/rv64/cpu.hpp>

void MIC::entry() {
    handler(RISCV::Context::push<RISCV::Machine>());
    RISCV::Context::pop<RISCV::Machine>();
}

void MIC::handler(void *args) {
    uintmax_t mcause = RISCV::csrr<RISCV::Machine::CAUSE>();
    bool is_interrupt = mcause >> (Traits<Machine>::XLEN - 1);
    int code = (mcause << 1) >> 1;

    if (is_interrupt) {
        switch (code) {
        case Interrupt::TIMER:
            if constexpr (Meta::SAME<RISCV::KernelMode,
                                     RISCV::Supervisor>::Result) {
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
            RISCV::Context *context = reinterpret_cast<RISCV::Context *>(args);
            context->pc += 4;
            Syscall::handler(reinterpret_cast<void *>(context->a0));
        } else {
            error(mcause);
        }
    }
}

void MIC::error(intmax_t mcause) {
    auto mstatus =
        reinterpret_cast<void *>(RISCV::csrr<RISCV::Machine::STATUS>());
    auto mepc = reinterpret_cast<void *>(RISCV::csrr<RISCV::Machine::EPC>());
    auto mtval = reinterpret_cast<void *>(RISCV::csrr<RISCV::Machine::TVAL>());
    ERROR(true,
          "Ohh it's a Trap!\nmcause: %d\nmepc: %p\nmtval: %p\nmstatus: %p\n",
          mcause, mepc, mtval, mstatus);
}

void SIC::entry() {
    RISCV::Context::push<RISCV::Supervisor>();
    SIC::handler();
    RISCV::Context::pop<RISCV::Supervisor>();
}

void SIC::handler() {
    uintmax_t scause = RISCV::csrr<RISCV::Supervisor::CAUSE>();
    bool is_interrupt = scause >> (Traits<Machine>::XLEN - 1);
    int code = (scause << 1) >> 1;
    auto core = RISCV::core();
    if (is_interrupt) {
        switch (code) {
        case Interrupt::TIMER:
            RISCV::syscall(RISCV::CLINT::reset);
            Timer::handler(core);
            break;
        }
    } else {
        error(scause);
    }
}

void SIC::error(intmax_t scause) {
    auto sepc = reinterpret_cast<void *>(RISCV::csrr<RISCV::Supervisor::EPC>());
    ERROR(true,
          "Ohh it's a Trap!\n"
          "scause: ",
          scause, "\nsepc: ", sepc, "\n");
}

void Syscall::handler(void *function) {
    auto core = RISCV::core();
    auto addr = reinterpret_cast<uintptr_t>(function);
    if (addr == reinterpret_cast<uintptr_t>(&RISCV::CLINT::reset)) {
        RISCV::CLINT::reset(core);
        RISCV::csrs<RISCV::Machine::IE>(RISCV::Machine::TI);
        RISCV::csrc<RISCV::Machine::IP>(RISCV::Supervisor::TI);
    } else {
        // TODO: THIS EXECUTE IN MACHINE MODE
        reinterpret_cast<void (*)()>(function)();
        ERROR(true, "Invalid Syscall!\n");
    }
}
