#include <cpu.hpp>
#include <definitions.hpp>
#include <io/logger.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <thread.hpp>
#include <timer/timer.hpp>

static char STACK[Machine::Memory::Page::SIZE];
extern int main(void *);
static Thread thread;

struct System {
    static void init() {
        CPU::disable_interrupts();
        Logger::init();
        Logger::log("\nQ U A R K | [μSystem]\n");
        Memory::init();
        Thread::create(&thread, main, Thread::Priority::NORMAL);

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
    CPU::Context::pc(CPU::Trap::pc());

    __asm__ volatile("csrr t0, mcause\nli t1, %0\nand t0, t0, t1\nbne t0, zero, _Z17interrupt_handlerv" ::"i"(1L << (Machine::XLEN - 1))
                     : "t0", "t1");

    Logger::log("Ohh it's a Trap!\n");
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    Logger::log("mcause: %p\n", mcause);
    Logger::log("mepc: %p\n", mepc);
    Logger::log("mtval: %p\n", mtval);

    if ((mcause >> (Machine::XLEN - 1)) == 0) {
        Logger::log("Exception Detected!\n");
    } else {
        Logger::log("Interruption Detected!\n");
    }
    while (1);
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::trap(ktrap);

    if (CPU::id() == 0) {
        CPU::stack(STACK + Machine::Memory::Page::SIZE);
        System::init();
    } else {
        CPU::idle();
    }
}
