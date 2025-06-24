#include <cpu.hpp>
#include <definitions.hpp>
#include <io/logger.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <thread.hpp>
#include <timer/timer.hpp>

static char STACK[Machine::Memory::Page::SIZE * 2];

struct Kernel {
    static void init() {
        CPU::disable_interrupts();
        Logger::init();
        Logger::log("\nQ U A R K | [μKernel]\n");
        Memory::init();
        Logger::log("Done!\n");
        Timer::init();
        Thread::init();
        CPU::idle();
    }
};

void interrupt_handler() {
    CPU::Trap::Interrupt::Type type = CPU::Trap::Interrupt::type();
    switch (type) {
        case CPU::Trap::Interrupt::TIMER:
            Timer::reset();
            Thread::reschedule();
            break;
    }
    while (1);
}

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::disable_interrupts();
    CPU::Context::save();
    CPU::Context::pc(CPU::Trap::ra());

    if (CPU::Trap::kind() == CPU::Trap::INTERRUPT) {
        interrupt_handler();
    } else {
        Logger::log("Ohh it's a Trap!\n");
        uintptr_t mcause, mepc, mtval;
        __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
        __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
        __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
        Logger::log("mcause: %p\n", mcause);
        Logger::log("mepc: %p\n", mepc);
        Logger::log("mtval: %p\n", mtval);
    }
    while (1);
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::trap(ktrap);

    if (CPU::id() == 0) {
        CPU::stack(STACK + Machine::Memory::Page::SIZE);
        Kernel::init();
    } else {
        CPU::idle();
    }
}
