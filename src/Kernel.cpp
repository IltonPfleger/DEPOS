#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

extern "C" char __BOOT_STACK__[];

struct Kernel {
    static void init() {
        if (CPU::core() == 0) {
            Logger::init();
            Logger::println("\nQ U A R K | [μKernel]\n");
            Memory::init();
            // Timer::init();
            Logger::println("Done!\n");
            Thread::init();
            Thread::core();
        }
        Logger::println("%d\n", CPU::core());
        // Thread::core();
        for (;;);
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Interrupt::disable();
    CPU::Context::push<true>();
    Thread::running()->context = CPU::Context::get();

    if (CPU::Trap::type() == CPU::Trap::Type::EXCEPTION) {
        uintptr_t mcause, mepc, mtval;
        __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
        __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
        __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
        Logger::println("Ohh it's a Trap!\n");
        Logger::println("mcause: %p\n", mcause);
        Logger::println("mepc: %p\n", mepc);
        Logger::println("mtval: %p\n", mtval);
        while (1);
    }

    switch (CPU::Interrupt::type()) {
        case CPU::Interrupt::Type::TIMER:
            // Timer::handler();
            break;
    }

    CPU::Context::set(Thread::running()->context);
    CPU::Context::pop();
    CPU::iret();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::Interrupt::disable();
    CPU::Stack::set(__BOOT_STACK__ + (CPU::core() * Traits<Memory>::Page::SIZE));
    CPU::Trap::set(ktrap);
    Kernel::init();
}
