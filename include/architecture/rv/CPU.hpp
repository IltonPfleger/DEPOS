#pragma once

#include <architecture/common/Atomic.hpp>
#include <architecture/rv/Context.hpp>
#include <architecture/rv/Interruptions.hpp>
#include <architecture/rv/Modes.hpp>
#include <memory/Memory.hpp>
#include <utils/Console.hpp>

namespace rv {
class CPU {
  public:
    using Context = ContextBase<rv::KernelMode>;
    using Interruptions = rv::Interruptions;
    using Atomic = ArchitectureCommon::Atomic;

    static auto idle() { asm("wfi"); }
    static auto halt() { asm("1: wfi\n j 1b"); }
    static auto syscall(auto f) { asm("mv a0, %0\necall" ::"r"(f)); }
    static auto id() {
        unsigned long tp;
        asm("mv %0, tp" : "=r"(tp));
        return tp;
    }

    __attribute__((naked)) static void init() {
        unsigned int core;

        asm("csrw mscratch, ra");

        // Disable Interruptions
        asm("csrc mstatus, 0x8");

        // Halt Cores That Don't Support Supervisor Mode If Enabled
        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            asm("csrr a0, misa\n"
                "and a0, a0, %0\n"
                "bnez a0, 1f\n"
                "wfi\n"
                "1:" ::"r"(1ULL << ('S' - 'A')));
        }

        // Use TP as MHARTID
        asm("csrr tp, mhartid\n"
            "mv %0, tp"
            : "=r"(core));

        asm("mv sp, %0" ::"r"(Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::PageSize * core));

        if (id() == Traits<CPUS>::BSP) {
            __bmm.start = Traits<MemoryMap>::RamEnd - Traits<Memory>::PageSize * Traits<CPUS>::COUNT;
            __bmm.end = Traits<MemoryMap>::RamEnd;
        }

        asm("csrr ra, mscratch\n"
            "ret");
    }

    static void barrier(unsigned int cores = Traits<CPUS>::ACTIVE) {
        static volatile bool gsense = true;
        __attribute__((section(".data"))) static volatile unsigned int ready[2] = {0};

        bool sense = gsense;
        unsigned int arrived = Atomic::finc(ready[sense]);

        if (arrived == cores - 1) {
            ready[sense] = 0;
            gsense = !sense;
        } else {
            while (gsense == sense)
                ;
        }
    }
};
} // namespace rv
