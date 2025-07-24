#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

static char STACK[Machine::CPUS][Traits::Memory::Page::SIZE];

struct Kernel {
    static void init() {
        if (CPU::core() == 0) {
            Logger::init();
            Logger::println("\nQ U A R K | [μKernel]\n");
            Memory::init();
            Logger::println("Done!\n");
        }
        Thread::init();
        CPU::Interrupt::disable();
        Timer::init();
        Thread::go();
    }

    static void trap() {
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
                Timer::handler();
        }
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Interrupt::disable();
    CPU::Context::push<true>();
    Thread::running()->context = CPU::Context::get();
    Kernel::trap();
    CPU::Context::set(Thread::running()->context);
    CPU::Context::pop();
    CPU::iret();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::Interrupt::disable();
    CPU::Trap::set(ktrap);
    CPU::Stack::set(STACK[CPU::core()] + Traits::Memory::Page::SIZE);
    Kernel::init();
}
