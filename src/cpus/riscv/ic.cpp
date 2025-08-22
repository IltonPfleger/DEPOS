#include <IO/Logger.hpp>
#include <cpus/riscv/cpu.hpp>

namespace Timer {
    void handler();
}

__attribute__((naked, aligned(4))) void MIC::entry() {
    RISCV::Context::push<RISCV::Machine>();
    handler();
    RISCV::Context::pop<RISCV::Machine>();
}

void MIC::handler() {
    auto cause        = RISCV::csrr<RISCV::Machine::CAUSE>();
    bool is_interrupt = cause >> (Traits::Machine::XLEN - 1);
    auto code         = (cause << 1) >> 1;
    Logger::println("X\n");

    if (is_interrupt) {
        switch (code) {
            case 7:
                auto core = RISCV::core();
                RISCV::CLINT::reset(core);
                Timer::handler();
        }
    } else {
        error();
    }
}

void MIC::error() {
    // uintptr_t mcause, mepc, mtval;
    //__asm__ volatile("csrr %0, scause" : "=r"(mcause));
    //__asm__ volatile("csrr %0, sepc" : "=r"(mepc));
    //__asm__ volatile("csrr %0, stval" : "=r"(mtval));
    // ERROR(true,
    //       "Ohh it's a Trap!\n"
    //       "mcause: %p\n"
    //       "mepc: %p\n"
    //       "mtval: %p\n",
    //       mcause, mepc, mtval);
}
