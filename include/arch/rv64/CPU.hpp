#pragma once
class CPU {
  public:
    using Context = RISCV::Context;
    using Interruptions = RISCV::Interruptions;
    using Atomic = RISCV::Atomic;
    class TLB {
      public:
        static auto flush() { asm volatile("sfence.vma zero, zero"); }
    };
    static auto idle() { asm volatile("wfi"); }
    static auto syscall(auto f) { asm volatile("mv a0, %0\necall" ::"r"(f)); }
    static auto id() {
        unsigned long tp;
        asm volatile("mv %0, tp" : "=r"(tp));
        return tp;
    }

    __attribute__((naked)) static void setup() {
        unsigned long core;
        unsigned long stacksz = Traits<Memory>::PAGE_SIZE;
        asm volatile("csrr tp, mhartid\nmv %0, tp" : "=r"(core));
        uintptr_t addr = Traits<MemoryMap>::RAM_END - stacksz * core;
        asm volatile("mv sp, %0\nret" ::"r"(addr));
    }

    __attribute__((noinline)) static void init() {
        static_assert(!Traits<System>::MULTITASK ||
                      Meta::SAME<KernelMode, SupervisorMode>::Result);

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            if (CPU::id() == 0) {
                for (;;)
                    CPU::idle();
            }
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<MachineMode::IE>(MachineMode::TI);
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            csrw<SupervisorMode::TVEC>(SIC::entry);
            csrw<MachineMode::TVEC>(MIC::entry);
            csrw<MachineMode::MIDELEG>(0x222);
            csrw<MachineMode::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<MachineMode::PMPCFG0>(0b11111);
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
        } else {
            csrs<MachineMode::STATUS>(MachineMode::ME2ME);
            csrw<MachineMode::TVEC>(MIC::entry);
        }

        csrw<MachineMode::EPC>(__builtin_return_address(0));
        MachineMode::ret();
    }
};
