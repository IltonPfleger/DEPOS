#include <cpu.hpp>
#include <definitions.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <thread.hpp>

struct System {
    static void init() {
        CPU::disable_interrupts();
        IO::init();
        IO::out("\nQ U A R K | [μSystem]\n");
        Memory::init();
        Thread main;
        // Thread::init();
        // Thread teste;
        // Thread::ready.push(teste);
        Memory::Heap heap;
        void *p1 = heap.malloc(16);
        void *p2 = heap.malloc(16);
        void *p3 = heap.malloc(16);
        heap.free(p1);
        heap.free(p2);
        heap.free(p3);

        // void* mem  = Memory::kmalloc();
        // void* mem2 = Memory::kmalloc();
        // Memory::kfree(mem);
        // Memory::kfree(mem2);
        //  mem  = Memory::malloc(25);
        //  mem2 = Memory::malloc(25);
        //  Memory::free(mem, 25);
        //  Memory::free(mem2, 25);

        IO::out("Done!\n");
        __asm__ volatile(".word 0xffffffff");
        CPU::enable_interrupts();
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::Context::save();
    // CPU::stack(kThreads[CPU::id()].stack);
    // CPU::context(&kThreads[CPU::id()].context);

    IO::out("Ohh it's a Trap!\n");
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    IO::out("mcause: %p\n", mcause);
    IO::out("mepc: %p\n", mepc);
    IO::out("mtval: %p\n", mtval);

    if ((mcause >> (Machine::XLEN - 1)) == 0) {
        IO::out("Exception Detected!\n");
    } else {
        IO::out("Interruption Detected!\n");
    }

    CPU::Context::load();
    CPU::idle();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::trap(ktrap);

    if (CPU::id() == 0) {
        CPU::set_stack((void *)0x80200000);
        System::init();
    } else {
        CPU::idle();
    }
}
