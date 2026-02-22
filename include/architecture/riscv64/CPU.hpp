#pragma once

#include <architecture/common/Atomic.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Traits.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <memory/MemoryMap.hpp>

namespace riscv64 {
class CPU {
  public:
    using Context = ContextBase<KernelMode>;
    using Atomic = ArchitectureCommon::Atomic;

    static unsigned int be32toh(unsigned int x) {
        return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
    }

    static uint32_t htobe32(uint32_t x) {
        return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) | ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
    }

    static uint16_t htobe16(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }

    static uint16_t be16toh(uint16_t x) { return (uint16_t)((x >> 8) | (x << 8)); }

    static auto idle() { asm volatile("wfi"); }
    static auto halt() { asm volatile("1: wfi\n j 1b"); }

    static void syscall() { asm volatile("ecall"); }

    static auto stack() {
        unsigned char *sp;
        asm volatile("mv %0, sp" : "=r"(sp));
        return sp;
    }

    static unsigned int id() {
        if constexpr (!Traits<RISCV>::Supervisor)
            return csrr<MachineMode::HARTID>() - Traits<::CPU>::Offset;
        else {
            register unsigned int core asm("tp");
            return core;
        }
    }

    __attribute__((naked)) static void init() {
        register unsigned int core asm("tp");

        // Disable Interruptions
        asm volatile("csrc mstatus, 0x8");

        // Save Return Address
        asm volatile("csrw mscratch, ra");

        // Use Thread Pointer as Core ID
        asm volatile("csrr tp, mhartid");

        if (core < Traits<::CPU>::Offset) {
            halt();
        }

        core -= Traits<::CPU>::Offset;

        if (core >= Traits<::CPU>::Active) {
            halt();
        }

        // Get A Stack
        asm volatile("mv sp, %0" ::"r"(Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::StackSize * core));

        // Setup Boot Memory
        if (id() == Traits<::CPU>::BSP) {
            __bmm.start = Traits<MemoryMap>::RamEnd - Traits<Memory>::StackSize * Traits<::CPU>::Active - 1;
            __bmm.end = Traits<MemoryMap>::RamEnd;
        }

        // Restore Return Address
        asm volatile("csrr ra, mscratch");

        // Return
        asm volatile("ret");
    }

    static void mb() { asm volatile("" ::: "memory"); }

    static void barrier() {
        __attribute__((section(".barrier"))) static volatile unsigned char gsense = 0;
        __attribute__((section(".barrier"))) static volatile unsigned int ready[2] = {0};

        unsigned char sense = Atomic::load(gsense);
        unsigned int arrived = Atomic::finc(ready[sense]);

        if (arrived == Traits<::CPU>::Active - 1) {
            Atomic::store(ready[sense], 0);
            Atomic::store(gsense, !sense);
        } else {
            while (Atomic::load(gsense) == sense)
                ;
        }
    }

    class Interruptions {
      public:
        static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }
        static bool disable() {
            unsigned long status = csrrc<KernelMode::STATUS>(KernelMode::IRQE);
            return (status & KernelMode::IRQE) != 0;
        }
    };
};
} // namespace riscv64
