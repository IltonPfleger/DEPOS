#ifndef KERNEL_HPP
#define KERNEL_HPP
#include <CPU.hpp>
// #include <memory.h>
#include <utils/welcome.h>

#include <IO/IO.hpp>
#include <IO/UART.hpp>

using UART0 = UART<0x10000000, 24000000, 115200>;

void kmain() {
    if (CPU::id() == 0) {
        CPU::begin_atomic();
        IO<UART0>::init();
        IO<UART0>::print(WELCOME);
        //  memory_init();
        CPU::end_atomic();
    } else {
        CPU::idle();
    }
    CPU::idle();
}

__attribute__((naked)) void ktrap() {
    // cpu_scontext();
    //  kprint("EXCEPTION\n");
    // cpu_lcontext();
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
#endif
