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

    static void syscall(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6,
                        uint64_t a7) {
        register uint64_t r0 asm("a0") = a0;
        register uint64_t r1 asm("a1") = a1;
        register uint64_t r2 asm("a2") = a2;
        register uint64_t r3 asm("a3") = a3;
        register uint64_t r4 asm("a4") = a4;
        register uint64_t r5 asm("a5") = a5;
        register uint64_t r6 asm("a6") = a6;
        register uint64_t r7 asm("a7") = a7;
        asm volatile("ecall" ::"r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r6), "r"(r7) : "memory");
    }

    static auto stack() {
        unsigned char *sp;
        asm volatile("mv %0, sp" : "=r"(sp));
        return sp;
    }

    static unsigned int id() {
        if constexpr (!Traits<RISCV>::Supervisor)
            return csrr<MachineMode::HARTID>();
        else {
            unsigned int tp;
            asm volatile("mv %0, tp" : "=r"(tp));
            return tp;
        }
    }

    __attribute__((naked)) static void init() {
        unsigned int core;

        // Disable Interruptions
        asm volatile("csrc mstatus, 0x8");

        // Save Return Address
        asm volatile("csrw mscratch, ra");

        // Ensure ISA Compliance: halt cores lacking Supervisor Mode (S-mode) support.
        if constexpr (Traits<RISCV>::Supervisor) {
            asm volatile("csrr a0, misa\n"
                         "and a0, a0, %0\n"
                         "bnez a0, 2f\n"
                         "1: wfi\n"
                         "j 1b\n"
                         "2:" ::"r"(1ULL << ('S' - 'A')));
        }

        // Use Thread Pointer as Core ID
        asm volatile("csrr tp, mhartid\n"
                     "addi tp, tp, %[offset]\n"
                     "mv %[core], tp"
                     : [core] "=r"(core)
                     : [offset] "i"(Traits<::CPU>::Active - Traits<::CPU>::Count));

        // Get A Stack
        asm volatile("mv sp, %0" ::"r"(Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::PageSize * core));

        // Setup Boot Memory
        if (id() == Traits<::CPU>::BSP) {
            __bmm.start = Traits<MemoryMap>::RamEnd - Traits<Memory>::PageSize * Traits<::CPU>::Active - 1;
            __bmm.end = Traits<MemoryMap>::RamEnd;
        }

        // Wait Here For All See The Memory Pool
        barrier();

        // Restore Return Address
        asm volatile("csrr ra, mscratch");

        asm volatile("ret");
    }

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
