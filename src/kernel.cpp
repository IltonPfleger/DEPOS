#include <cpu.hpp>
#include <definitions.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <thread.hpp>

char STACK[Machine::Memory::Page::SIZE * 5];

int teste0(void *) {
	int i = 0;
    while (i < 10) {
        IO::out("Thread0 %d\n", i);
        Thread::yield();
		i++;
    }
    // IO::out("BACK0\n");
    // while (1);
    return 0;
}

int teste1(void *) {
	int i = 0;
    while (i < 10) {
        IO::out("Thread1 %d\n", i);
        Thread::yield();
		i++;
    }
    // IO::out("BACK1\n");
    // while (1);
    return 0;
}

struct System {
    static void init() {
        uintptr_t mstatus;
        __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
        mstatus |= (3 << 11);
        __asm__ volatile("csrw mstatus, %0" ::"r"(mstatus));

        CPU::disable_interrupts();
        IO::init();
        IO::out("\nQ U A R K | [μSystem]\n");
        Memory::init();

        Memory::Heap SYSTEM;
        int *p1 = new (SYSTEM) int;
        int *p2 = new (SYSTEM) int;
        operator delete(p1, SYSTEM);
        operator delete(p2, SYSTEM);

        Thread main;
        Thread::create(&main, teste0, Thread::Priority::NORMAL);
        Thread main2;
        Thread::create(&main2, teste1, Thread::Priority::NORMAL);

        Thread::_running        = Thread::_ready.get();
        Thread::_running->state = Thread::RUNNING;
        CPU::Context::set((void *)0x80200000);
        CPU::Context::change(const_cast<CPU::Context *>(&Thread::_running->context));

        IO::out("Done!\n");
        //__asm__ volatile(".word 0xffffffff");
        CPU::enable_interrupts();
        CPU::idle();
    }
};

__attribute__((naked, aligned(4))) void ktrap() {
    // CPU::Context::save();
    //  CPU::stack(kThreads[CPU::id()].stack);
    //  CPU::context(&kThreads[CPU::id()].context);

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

    // CPU::Context::load();
    CPU::idle();
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
