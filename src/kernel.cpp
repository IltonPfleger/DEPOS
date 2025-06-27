import Definitions;
import CPU;
import Thread;
import Memory;
import Logger;
import Timer;

static char STACK[Machine::Memory::Page::SIZE/4];

struct Kernel {
    static void init() {
        CPU::Interrupt::disable();
        Logger::init();
        Logger::log("\nQ U A R K | [μKernel]\n");
        Memory::init();
        Logger::log("Done!\n");
        Timer::init();
        Thread::init();
        CPU::idle();
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Interrupt::disable();
    CPU::Context::push<true>();
    CPU::Context::save(const_cast<CPU::Context**>(&Thread::_running->context));

    if (CPU::Trap::type() == CPU::Trap::Type::INTERRUPT) {
        switch (CPU::Interrupt::type()) {
            case CPU::Interrupt::Type::TIMER:
                Timer::handler();
                break;
        }
    } else {
        Logger::log("Ohh it's a Trap!\n");
        uintptr_t mcause, mepc, mtval;
        __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
        __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
        __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
        Logger::log("mcause: %p\n", mcause);
        Logger::log("mepc: %p\n", mepc);
        Logger::log("mtval: %p\n", mtval);
        while (1);
    }

    CPU::Context::pop();
    CPU::iret();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::Trap::set(ktrap);

    if (CPU::id() == 0) {
        CPU::Stack::set(STACK + Machine::Memory::Page::SIZE);
        Kernel::init();
    } else {
        CPU::idle();
    }
}
