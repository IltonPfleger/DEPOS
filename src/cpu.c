#include <cpu.h>
#include <definitions.h>

int cpu_id() {
    int mhartid;
    __asm__ volatile("csrr %0, mhartid" : "=r"(mhartid));
    return mhartid;
}

void cpu_idle() { __asm__ volatile("1: wfi\nj 1b"); }

__attribute__((aligned(16))) uint8_t cpu_core_stacks[MACHINE_CPUS][MACHINE_MEMORY_PAGE_SIZE];
