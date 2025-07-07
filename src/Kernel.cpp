#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

static char STACK[Traits<Memory>::Page::SIZE];

struct Kernel {
    static void init() {
        Logger::init();
        Logger::log("\nQ U A R K | [μKernel]\n");
        Memory::init();
        Timer::init();
        Logger::log("Done!\n");
        Thread::init();
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Interrupt::disable();
    CPU::Context::push<true>();
    CPU::Context *context = CPU::Context::get();
    Thread::save(context);
    CPU::Stack::set(STACK + Traits<Memory>::Page::SIZE);

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

    CPU::Stack::set((char *)context);
    CPU::Context::pop();
    CPU::iret();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::Interrupt::disable();
    CPU::Trap::set(ktrap);

    if (CPU::id() == 0) {
        CPU::Stack::set(STACK + Traits<Memory>::Page::SIZE);
        Kernel::init();
    } else {
        for (;;);
    }
}
