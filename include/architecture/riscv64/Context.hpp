#pragma once

#include <architecture/riscv64/Modes.hpp>
#include <utils/string.hpp>

#pragma GCC push_options
#pragma GCC optimize("O0")

// TODO: Meta Template For CSRs
// TODO: TP For Linux And Supervisor BUG

namespace riscv64 {
template <typename T> class ContextBase {
  public:
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
    uint64_t status, scratch, pc;

    uint64_t &operator[](size_t i) { return (reinterpret_cast<uint64_t *>(&ra))[i - 1]; }

    ContextBase() = default;

    ContextBase(auto usp, auto ksp, auto pc, auto ra, auto a0) {
        this->ra = reinterpret_cast<uint64_t>(ra);
        this->pc = reinterpret_cast<uint64_t>(pc);
        this->status = static_cast<uint64_t>(T::ME2ME | T::PIRQE);
        this->a0 = reinterpret_cast<uint64_t>(a0);
        this->sp = reinterpret_cast<uint64_t>(usp);
        this->scratch = reinterpret_cast<uint64_t>(ksp);
    }

    template <typename F, typename... Args> __attribute__((naked)) void load(F f, Args... args) {
        asm volatile("mv s11, a0");
        f(args...);
        asm volatile("ld sp, %[sp](s11)" ::[sp] "i"(offsetof(ContextBase, sp)));
        asm volatile(
            "ld ra, %[ra](s11)\n"
            "ld gp, %[gp](s11)\n"
            "ld s0, %[s0](s11)\n"
            "ld a0, %[a0](s11)\n"
            "ld s1, %[s1](s11)\n"
            "ld s2, %[s2](s11)\n"
            "ld s3, %[s3](s11)\n"
            "ld s4, %[s4](s11)\n"
            "ld s5, %[s5](s11)\n"
            "ld s6, %[s6](s11)\n"
            "ld s7, %[s7](s11)\n"
            "ld s8, %[s8](s11)\n"
            "ld s9, %[s9](s11)\n"
            "ld s10, %[s10](s11)\n"
            "ld t0, %[pc](s11)\n"
            "ld t1, %[status](s11)\n"
            "ld t3, %[scratch](s11)\n"
            "csrw %[epcr], t0\n"
            "csrw %[statusr], t1\n"
            "csrw %[scratchr], t3\n"
            "ld s11, %[s11](s11)\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [s0] "i"(offsetof(ContextBase, s0)),
              [s1] "i"(offsetof(ContextBase, s1)), [a0] "i"(offsetof(ContextBase, a0)), [s2] "i"(offsetof(ContextBase, s2)),
              [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)), [s5] "i"(offsetof(ContextBase, s5)),
              [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)), [s8] "i"(offsetof(ContextBase, s8)),
              [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)), [s11] "i"(offsetof(ContextBase, s11)),
              [pc] "i"(offsetof(ContextBase, pc)), [status] "i"(offsetof(ContextBase, status)),
              [scratch] "i"(offsetof(ContextBase, scratch)), [epcr] "i"(T::EPC), [scratchr] "i"(T::SCRATCH),
              [statusr] "i"(T::STATUS));
        T::ret();
    }

    __attribute__((naked)) bool save() {
        asm volatile(
            "sd ra, %[ra](a0)\n"
            "sd gp, %[gp](a0)\n"
            "sd sp, %[sp](a0)\n"
            "sd zero,%[a0](a0)\n"
            "sd s0, %[s0](a0)\n"
            "sd s1, %[s1](a0)\n"
            "sd s2, %[s2](a0)\n"
            "sd s3, %[s3](a0)\n"
            "sd s4, %[s4](a0)\n"
            "sd s5, %[s5](a0)\n"
            "sd s6, %[s6](a0)\n"
            "sd s7, %[s7](a0)\n"
            "sd s8, %[s8](a0)\n"
            "sd s9, %[s9](a0)\n"
            "sd s10, %[s10](a0)\n"
            "sd s11, %[s11](a0)\n"
            "sd ra, %[pc](a0)\n"
            "csrr t0, %[scratchr]\n"
            "sd t0, %[scratch](a0)\n"
            "csrr t1, %[csrstatus]\n"
            "or t1, t1, %[me2me]\n"
            "sd t1, %[status](a0)\n"
            "li a0, 1\n"
            "ret"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [sp] "i"(offsetof(ContextBase, sp)),
              [a0] "i"(offsetof(ContextBase, a0)), [s0] "i"(offsetof(ContextBase, s0)), [s1] "i"(offsetof(ContextBase, s1)),
              [s2] "i"(offsetof(ContextBase, s2)), [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)),
              [s5] "i"(offsetof(ContextBase, s5)), [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)),
              [s8] "i"(offsetof(ContextBase, s8)), [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)),
              [s11] "i"(offsetof(ContextBase, s11)), [pc] "i"(offsetof(ContextBase, pc)), [csrstatus] "i"(T::STATUS),
              [scratch] "i"(offsetof(ContextBase, scratch)), [scratchr] "i"(T::SCRATCH), [me2me] "r"(T::ME2ME),
              [status] "i"(offsetof(ContextBase, status))
            : "memory");
    }

    template <bool ChangeStack = false> __attribute__((always_inline)) static inline ContextBase *push() {
        if constexpr (ChangeStack) {
            asm volatile("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }

        asm volatile(
            "addi sp, sp, %[size]\n"
            "sd ra, %[ra](sp)\n"
            "sd gp, %[gp](sp)\n"
            //"sd tp, %[tp](sp)\n"
            "sd t0, %[t0](sp)\n"
            "sd t1, %[t1](sp)\n"
            "sd t2, %[t2](sp)\n"
            "sd t3, %[t3](sp)\n"
            "sd t4, %[t4](sp)\n"
            "sd t5, %[t5](sp)\n"
            "sd t6, %[t6](sp)\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [tp] "i"(offsetof(ContextBase, tp)),
              [t0] "i"(offsetof(ContextBase, t0)), [t1] "i"(offsetof(ContextBase, t1)), [t2] "i"(offsetof(ContextBase, t2)),
              [t3] "i"(offsetof(ContextBase, t3)), [t4] "i"(offsetof(ContextBase, t4)), [t5] "i"(offsetof(ContextBase, t5)),
              [t6] "i"(offsetof(ContextBase, t6)), [size] "i"(-sizeof(ContextBase))
            : "memory");

        asm volatile(
            "sd a0, %[a0](sp)\n"
            "sd a1, %[a1](sp)\n"
            "sd a2, %[a2](sp)\n"
            "sd a3, %[a3](sp)\n"
            "sd a4, %[a4](sp)\n"
            "sd a5, %[a5](sp)\n"
            "sd a6, %[a6](sp)\n"
            "sd a7, %[a7](sp)\n"
            :
            : [a0] "i"(offsetof(ContextBase, a0)), [a1] "i"(offsetof(ContextBase, a1)), [a2] "i"(offsetof(ContextBase, a2)),
              [a3] "i"(offsetof(ContextBase, a3)), [a4] "i"(offsetof(ContextBase, a4)), [a5] "i"(offsetof(ContextBase, a5)),
              [a6] "i"(offsetof(ContextBase, a6)), [a7] "i"(offsetof(ContextBase, a7))
            : "memory");

        asm volatile(
            "sd s0,  %[s0](sp)\n"
            "sd s1,  %[s1](sp)\n"
            "sd s2,  %[s2](sp)\n"
            "sd s3,  %[s3](sp)\n"
            "sd s4,  %[s4](sp)\n"
            "sd s5,  %[s5](sp)\n"
            "sd s6,  %[s6](sp)\n"
            "sd s7,  %[s7](sp)\n"
            "sd s8,  %[s8](sp)\n"
            "sd s9,  %[s9](sp)\n"
            "sd s10, %[s10](sp)\n"
            "sd s11, %[s11](sp)\n"
            :
            : [s0] "i"(offsetof(ContextBase, s0)), [s1] "i"(offsetof(ContextBase, s1)), [s2] "i"(offsetof(ContextBase, s2)),
              [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)), [s5] "i"(offsetof(ContextBase, s5)),
              [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)), [s8] "i"(offsetof(ContextBase, s8)),
              [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)), [s11] "i"(offsetof(ContextBase, s11))
            : "memory");

        asm volatile("csrr t0, %[statusr]\n"
                     "csrr t1, %[epcr]\n"
                     "sd t0, %[statuso](sp)\n"
                     "sd t1, %[pc](sp)" ::[statusr] "i"(T::STATUS),
                     [epcr] "i"(T::EPC), [statuso] "i"(offsetof(ContextBase, status)), [pc] "i"(offsetof(ContextBase, pc)));
        register ContextBase *sp asm("sp");
        return sp;
    }

    template <bool ChangeStack = false> __attribute__((naked)) static void pop() {
        asm volatile("ld t0, %[statuso](sp)\n"
                     "ld t1, %[pc](sp)\n"
                     "csrw %[statusr], t0\n"
                     "csrw %[epcr], t1"
                     :
                     : [statusr] "i"(T::STATUS), [epcr] "i"(T::EPC), [pc] "i"(offsetof(ContextBase, pc)),
                       [statuso] "i"(offsetof(ContextBase, status))
                     : "t0");

        asm volatile(
            "ld ra, %[ra](sp)\n"
            "ld gp, %[gp](sp)\n"
            //"ld tp, %[tp](sp)\n"
            "ld t0, %[t0](sp)\n"
            "ld t1, %[t1](sp)\n"
            "ld t2, %[t2](sp)\n"
            "ld s0, %[s0](sp)\n"
            "ld s1, %[s1](sp)\n"
            "ld a0, %[a0](sp)\n"
            "ld a1, %[a1](sp)\n"
            "ld a2, %[a2](sp)\n"
            "ld a3, %[a3](sp)\n"
            "ld a4, %[a4](sp)\n"
            "ld a5, %[a5](sp)\n"
            "ld a6, %[a6](sp)\n"
            "ld a7, %[a7](sp)\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [tp] "i"(offsetof(ContextBase, tp)),
              [t0] "i"(offsetof(ContextBase, t0)), [t1] "i"(offsetof(ContextBase, t1)), [t2] "i"(offsetof(ContextBase, t2)),
              [s0] "i"(offsetof(ContextBase, s0)), [s1] "i"(offsetof(ContextBase, s1)), [a0] "i"(offsetof(ContextBase, a0)),
              [a1] "i"(offsetof(ContextBase, a1)), [a2] "i"(offsetof(ContextBase, a2)), [a3] "i"(offsetof(ContextBase, a3)),
              [a4] "i"(offsetof(ContextBase, a4)), [a5] "i"(offsetof(ContextBase, a5)), [a6] "i"(offsetof(ContextBase, a6)),
              [a7] "i"(offsetof(ContextBase, a7))
            : "memory");

        asm volatile(
            "ld s2,  %[s2](sp)\n"
            "ld s3,  %[s3](sp)\n"
            "ld s4,  %[s4](sp)\n"
            "ld s5,  %[s5](sp)\n"
            "ld s6,  %[s6](sp)\n"
            "ld s7,  %[s7](sp)\n"
            "ld s8,  %[s8](sp)\n"
            "ld s9,  %[s9](sp)\n"
            "ld s10, %[s10](sp)\n"
            "ld s11, %[s11](sp)\n"
            "ld t3,  %[t3](sp)\n"
            "ld t4,  %[t4](sp)\n"
            "ld t5,  %[t5](sp)\n"
            "ld t6,  %[t6](sp)\n"
            "addi sp, sp, %[size]\n"
            :
            : [s2] "i"(offsetof(ContextBase, s2)), [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)),
              [s5] "i"(offsetof(ContextBase, s5)), [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)),
              [s8] "i"(offsetof(ContextBase, s8)), [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)),
              [s11] "i"(offsetof(ContextBase, s11)), [t3] "i"(offsetof(ContextBase, t3)), [t4] "i"(offsetof(ContextBase, t4)),
              [t5] "i"(offsetof(ContextBase, t5)), [t6] "i"(offsetof(ContextBase, t6)), [size] "i"(sizeof(ContextBase))
            : "memory");

        if constexpr (ChangeStack) {
            asm volatile("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }

        T::ret();
    }
};

using MachineContext = ContextBase<MachineMode>;
using SupervisorContext = ContextBase<SupervisorMode>;
} // namespace riscv64

#pragma GCC pop_options
