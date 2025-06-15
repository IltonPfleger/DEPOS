#include <cpu.hpp>
#include <definitions.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>
#include <process.hpp>

static struct Process kProcesses[Machine::CPUS];

void kmain() {
    if (CPU::id() == 0) {
        CPU::begin_atomic();
        IO<UART>::init();
        IO<UART>::out("\nQ U A R K | [μKernel]\n");
        Memory::init();
        void* mem  = Memory::alloc(25);
        void* mem2 = Memory::alloc(25);
        Memory::free(mem, 25);
        Memory::free(mem2, 25);
        mem  = Memory::alloc(25);
        mem2 = Memory::alloc(25);
        Memory::free(mem, 25);
        Memory::free(mem2, 25);

        __asm__ volatile(".word 0xffffffff");
        // IO<UART0>::init();
        // Memory::init(reinterpret_cast<uintptr_t>(__KERNEL_END__));
        // Memory::alloc(13);
        IO<UART>::out("Done!\n");
        CPU::end_atomic();
    }
    CPU::idle();
}

__attribute__((naked, aligned(4))) void ktrap() {
    CPU::save();
    // CPU::load_stack_per_id();

    IO<UART>::out("Ohh it's a Trap!\n");
    uintptr_t mcause, mepc, mtval;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile("csrr %0, mtval" : "=r"(mtval));
    IO<UART>::out("mcause: %lx\n", mcause);
    IO<UART>::out("mepc: %lx\n", mepc);
    IO<UART>::out("mtval: %lx\n", mtval);

    if ((mcause >> (Machine::XLEN - 1)) == 0) {
        IO<UART>::out("Exception Detected!\n");
    } else {
        IO<UART>::out("Interruption Detected!\n");
    }

    CPU::load();
    CPU::idle();
}

__attribute__((naked, section(".boot"))) void kboot() {
    CPU::stack(kProcesses[CPU::id()].stack);
    CPU::context(kProcesses[CPU::id()].context);

    // trap handler.
    __asm__ volatile("csrw mtvec, %0" ::"r"(ktrap));

    // start.
    __asm__ volatile("jr %0" ::"r"(kmain));
}
