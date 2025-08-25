#include <IO/Debug.hpp>
#include <cpus/riscv/cpu.hpp>

__attribute__((naked, aligned(4))) void MIC::entry() {
    RISCV::Context* context = RISCV::Context::push<RISCV::Machine>();
    MIC::handler(context);
    RISCV::Context::pop<RISCV::Machine>();
}

void MIC::handler(void* args) {
    auto cause        = RISCV::csrr<RISCV::Machine::CAUSE>();
    bool is_interrupt = cause >> (Traits::Machine::XLEN - 1);
    auto code         = (cause << 1) >> 1;

    if (is_interrupt) {
        switch (code) {
            case 7:
                // auto core = RISCV::core();
                // RISCV::CLINT::reset(core);
                RISCV::csrc<RISCV::Machine::IE>(RISCV::Machine::TI);
                RISCV::csrs<RISCV::Machine::IP>(RISCV::Supervisor::TI);
                //  Timer::handler();
        }
    } else {
        RISCV::Context* context = reinterpret_cast<RISCV::Context*>(args);
        switch (cause) {
            case 9:
                context->pc += 4;
                Syscall::handler(static_cast<Syscall::Type>(context->a6));
                break;
            default:
                error();
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

void MIC::Syscall::handler(Type type) {
    switch (type) {
        case TIMER:
            auto core = RISCV::core();
            RISCV::CLINT::reset(core);
            Timer::handler(core);
            RISCV::csrc<RISCV::Machine::IP>(RISCV::Supervisor::TI);
            RISCV::csrs<RISCV::Machine::IE>(RISCV::Machine::TI);
    };
}

__attribute__((naked, aligned(4))) void SIC::entry() {
    RISCV::Context* context = RISCV::Context::push<RISCV::Supervisor>();
    SIC::handler(context);
    RISCV::Context::pop<RISCV::Supervisor>();
}

void SIC::handler(void*) {
    auto cause        = RISCV::csrr<RISCV::Supervisor::CAUSE>();
    bool is_interrupt = cause >> (Traits::Machine::XLEN - 1);
    auto code         = (cause << 1) >> 1;

    if (is_interrupt) {
        switch (code) {
            case 5:
                asm volatile("li a6, %0\necall" ::"i"(MIC::Syscall::TIMER));
                // auto core = RISCV::core();
                // RISCV::CLINT::reset(core);
                /// RISCV::csrc<RISCV::Machine::IE>(RISCV::Machine::TI);
                /// RISCV::csrs<RISCV::Machine::IP>(RISCV::Supervisor::TI);
                //  Timer::handler();
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
