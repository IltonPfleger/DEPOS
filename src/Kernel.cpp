#include <CPU.hpp>
#include <IO/Debug.hpp>
#include <Language.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

static char STACK[Machine::CPUS][Traits::Memory::Page::SIZE];

struct Kernel {
    static void init() {
        if (CPU::core() == 0) {
            // Language::init();
            Logger::init();
            Logger::println("\nQ U A R K | [μKernel]\n");
            Memory::init();
            Logger::println("Done!\n");
        }
        if (CPU::core() < Machine::CPUS) {
            Thread::init();
            Timer::init();
            Thread::run();
        }
        for (;;);
    }

    static void trap() {
        if (CPU::Trap::type() == CPU::Trap::Type::EXCEPTION) {
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

        switch (CPU::Interrupt::type()) {
            case CPU::Interrupt::Type::TIMER:
                Timer::handler();
        }
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Interrupt::disable();
    CPU::Context::push<true>();
    Kernel::trap();
    CPU::Context::pop();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::Interrupt::disable();
    CPU::Trap::set(ktrap);
    CPU::Stack::set(STACK[CPU::core()] + Traits::Memory::Page::SIZE);
    Kernel::init();
}

void cPlusPlus() {}
