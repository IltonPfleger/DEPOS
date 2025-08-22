#include <CPU.hpp>
#include <IO/Debug.hpp>
#include <Language.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Thread.hpp>
#include <Timer.hpp>

static volatile bool boot = true;

namespace Kernel {
    void init() {
        CPU::Interrupt::disable();
        if (CPU::core() == 0) {
            Logger::init();
            Logger::println("\n[Kernel]\n");
            Memory::init();
            Thread::init();
            Logger::println("Done!\n");
            boot = false;
        }
        while (boot);
        if (CPU::core() < Machine::CPUS) {
            Timer::init();
            Thread::run();
        }
        for (;;);
    }

    __attribute__((naked, aligned(4))) void ktrap() {
        CPU::Context::push();
        CPU::Trap::handler();
        CPU::Context::pop();
    }

    void exception() {
        // uintptr_t mcause, mepc, mtval;
        //__asm__ volatile("csrr %0, mcause" : "=r"(mcause));
        //__asm__ volatile("csrr %0, mepc" : "=r"(mepc));
        //__asm__ volatile("csrr %0, mtval" : "=r"(mtval));
        // ERROR(true,
        //       "Ohh it's a Trap!\n"
        //       "mcause: %p\n"
        //       "mepc: %p\n"
        //       "mtval: %p\n",
        //       mcause, mepc, mtval);
    }
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::init();
    Kernel::init();
}
