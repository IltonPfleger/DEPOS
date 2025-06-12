#ifndef KERNEL_HPP
#define KERNEL_HPP
#include <utils/welcome.h>

#include <CPU.hpp>
#include <IO/IO.hpp>
#include <IO/UART.hpp>
#include <memory.hpp>

using UART0 = UART<0x10000000, 32000000, 115200>;
extern "C" char __KERNEL_END__[];

void kmain() {
    if (CPU::id() == 0) {
        CPU::begin_atomic();
        IO<UART0>::init();
        Memory::init(reinterpret_cast<uintptr_t>(__KERNEL_END__));
        Memory::alloc(13);
        IO<UART0>::print(WELCOME);
        CPU::end_atomic();
    } else {
        CPU::idle();
    }
    CPU::idle();
}

__attribute__((naked)) void ktrap() {
    IO<UART0>::print("Trap!\n");
    while (1);
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
