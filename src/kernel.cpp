#include <cpu.hpp>
#include <io/io.hpp>
#include <io/uart.hpp>

void kmain() {
    if (CPU::id() == 0) {
        CPU::begin_atomic();
        IO<UART>::init();
        IO<UART>::out("\nQ U A R K | μKernel\n");
        // IO<UART0>::init();
        // Memory::init(reinterpret_cast<uintptr_t>(__KERNEL_END__));
        // Memory::alloc(13);
        CPU::end_atomic();
    } else {
        CPU::idle();
    }
    CPU::idle();
}

__attribute__((naked)) void ktrap() {
    CPU::save();
    IO<UART>::out("\nIt's a Trap!");
    __asm__ volatile("j .");
    CPU::load();

    // IO<UART0>::print("Trap!\n");
    // while (1);
    /* INTERRUPT OR EXCEPTION */
    //__asm__ volatile("csrr t0, mcause\nsrli t0, t0, %0" ::"i"(XLEN - 1));
    //__asm__ goto("beq t0, zero, %l[EXCEPTION]\nj %l[INTERRUPT]" ::
    //::EXCEPTION, INTERRUPT);

    // EXCEPTION:
    //     kprint("EXCEPTION\n");
    //     goto EXIT;
    // INTERRUPT:
    //     kprint("INTERRUPT\n");
    //     goto EXIT;
    // EXIT:
    //     __asm__ volatile("j .");
}
