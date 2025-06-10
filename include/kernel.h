#ifndef KERNEL_H
#define KERNEL_H
#include <utils/kprint.h>
#include <utils/welcome.h>
#include <memory.h>
#include <cpu.h>

void kmain() {
    if (cpu_id() == 0) {
        kprint_init();
        memory_init();
        kprint(WELCOME);
    } else {
        __asm__ volatile("sleep: wfi\nj sleep");
    }
}

__attribute__((naked)) void ktrap() {
    cpu_scontext();
    kprint("EXCEPTION\n");
    cpu_lcontext();
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
