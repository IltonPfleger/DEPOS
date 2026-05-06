#pragma once

#include <BootInformation.hpp>
#include <Traits.hpp>
#include <architecture/Endian.hpp>
#include <architecture/common/CPU.hpp>
#include <architecture/riscv64/ContextHandler.hpp>
#include <architecture/riscv64/CoreContextHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <memory/Heap.hpp>

namespace DEPOS {

namespace riscv64 {

class CPU : public ArchitectureCommon::CPU {
  public:
    static constexpr bool Supervisor  = Traits<RISCV>::Supervisor;
    static constexpr bool Virtualized = Traits<Application>::Virtualized;

    using NotSupervisorContext = Meta::IF<!Virtualized, DEPOS::riscv64::MachineContext<>, HypervisorContext>::Result;
    using Context              = Meta::IF<Supervisor, SupervisorContext<>, NotSupervisorContext>::Result;

    static void halt() { asm("csrc mstatus, 0x8; 1: wfi; j 1b"); }
    static auto idle() { asm("wfi"); }
    static void syscall() { asm("ecall"); }
    static void mb() { asm("fence iorw, iorw" ::: "memory"); }
    static uint64_t htobe64(uint64_t x) { return Endian::le2be64(x); }
    static uint32_t htobe32(uint32_t x) { return Endian::le2be32(x); }
    static uint16_t htobe16(uint16_t x) { return Endian::le2be16(x); }
    static uint64_t be64toh(uint64_t x) { return htobe64(x); }
    static uint32_t be32toh(uint32_t x) { return htobe32(x); }
    static uint16_t be16toh(uint16_t x) { return htobe16(x); }

    template <bool M = !Supervisor> static size_t id() {
        if constexpr (M) return csrr<MachineMode::HARTID>() - Traits<CPU>::Offset;
        return CoreContextHandler<SupervisorMode>::cpu();
    }

    class Interrupt {
      public:
        static void enable() {
            mb();
            csrs<KernelMode::STATUS>(KernelMode::IRQE);
        }

        static bool disable() {
            mb();
            unsigned long status = csrrc<KernelMode::STATUS>(KernelMode::IRQE);
            return (status & KernelMode::IRQE) != 0;
        }
    };

    __attribute__((naked, optimize("O0"))) static void init() {
        size_t core;
        uintptr_t position;

        // Disable Interruptions
        asm("csrc mstatus, 0x8");

        // Save Return Address
        asm("csrw mscratch, ra");

        // Get Which Core It's Running
        asm("csrr %0, mhartid" : "=r"(core));

        if (core < Traits<CPU>::Offset) {
            asm("wfi");
        }

        core -= Traits<CPU>::Offset;

        if (core >= Traits<CPU>::Active) {
            asm("wfi");
        }

        // Get The Code Position For Position Independent Environments
        asm("auipc %0, 0" : "=r"(position));
        position &= ~((1ULL << 30) - 1);

        // Get The Boot Stack
        asm("mv sp, %0" ::"r"(position + Traits<Memory>::Size - (Traits<Memory>::StackSize * core)));

        // Setup System Boot Info
        if (core == Traits<CPU>::BSP) {
            new (&__amm) decltype(__amm)(position, Traits<Memory>::Size);
            new (&__bmm) decltype(__bmm)(Traits<MemoryMap>::RamEnd, Traits<Memory>::StackSize * Traits<CPU>::Active);
        }

        barrier();

        // Restore Return Address
        asm("csrr ra, mscratch");

        // Return
        asm("ret");
    }
};

} // namespace riscv64

} // namespace DEPOS
