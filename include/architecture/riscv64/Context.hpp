#pragma once

#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/csrs.hpp>

// TODO: TP For Linux And Supervisor BUG

namespace DEPOS {

namespace riscv64 {

class Context {
  public:
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
    uint64_t status, scratch, cause, tval, pc;

    uint64_t &operator[](size_t i) { return (reinterpret_cast<uint64_t *>(&ra))[i - 1]; }
};

template <typename T> class ContextHandler : public Context {
  public:
    ContextHandler() = default;

    ContextHandler(auto usp, auto ksp, auto pc, auto ra, auto a0) {
        this->ra      = reinterpret_cast<uint64_t>(ra);
        this->pc      = reinterpret_cast<uint64_t>(pc);
        this->status  = static_cast<uint64_t>(T::ME2ME | T::PIRQE);
        this->a0      = reinterpret_cast<uint64_t>(a0);
        this->sp      = reinterpret_cast<uint64_t>(usp);
        this->scratch = reinterpret_cast<uint64_t>(ksp);
    }

    template <typename Function, typename... Args>
    __attribute__((naked)) static void swap(ContextHandler *previous, ContextHandler *next, Function f, Args... args) {
        previous->save();
        next->load(f, args...);
    }

    template <typename F, typename... Args> __attribute__((naked)) void load(F f, Args... args) {
        asm("ld sp, %0(a0)" ::[sp] "i"(offsetof(ContextHandler, sp)));
        asm("addi sp, sp, %0" ::"i"(-sizeof(ContextHandler)));
        f(args...);
        asm("addi sp, sp, %0" ::"i"(sizeof(ContextHandler)));

        asm("mv s11, %0" ::"r"(this));
        asm("ld t0, %0(s11)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, status)), "i"(T::STATUS));
        asm("ld t0, %0(s11)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, pc)), "i"(T::EPC));
        asm("ld t0, %0(s11)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, scratch)), "i"(T::SCRATCH));

        asm("ld ra, %[ra](s11)\n"
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
            "ld s11, %[s11](s11)\n"
            :
            : [ra] "i"(offsetof(ContextHandler, ra)), [gp] "i"(offsetof(ContextHandler, gp)),
              [s0] "i"(offsetof(ContextHandler, s0)), [s1] "i"(offsetof(ContextHandler, s1)),
              [a0] "i"(offsetof(ContextHandler, a0)), [s2] "i"(offsetof(ContextHandler, s2)),
              [s3] "i"(offsetof(ContextHandler, s3)), [s4] "i"(offsetof(ContextHandler, s4)),
              [s5] "i"(offsetof(ContextHandler, s5)), [s6] "i"(offsetof(ContextHandler, s6)),
              [s7] "i"(offsetof(ContextHandler, s7)), [s8] "i"(offsetof(ContextHandler, s8)),
              [s9] "i"(offsetof(ContextHandler, s9)), [s10] "i"(offsetof(ContextHandler, s10)),
              [s11] "i"(offsetof(ContextHandler, s11)));
        T::ret();
    }

    __attribute__((always_inline)) void save() {
        asm("sd ra,  %[ra](%[self])\n"
            "sd gp,  %[gp](%[self])\n"
            "sd sp,  %[sp](%[self])\n"
            "sd s0,  %[s0](%[self])\n"
            "sd s1,  %[s1](%[self])\n"
            "sd s2,  %[s2](%[self])\n"
            "sd s3,  %[s3](%[self])\n"
            "sd s4,  %[s4](%[self])\n"
            "sd s5,  %[s5](%[self])\n"
            "sd s6,  %[s6](%[self])\n"
            "sd s7,  %[s7](%[self])\n"
            "sd s8,  %[s8](%[self])\n"
            "sd s9,  %[s9](%[self])\n"
            "sd s10, %[s10](%[self])\n"
            "sd s11, %[s11](%[self])\n"
            "sd ra,  %[pc](%[self])\n"
            :
            : [self] "r"(this), [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)),
              [sp] "i"(offsetof(Context, sp)), [a0] "i"(offsetof(Context, a0)), [s0] "i"(offsetof(Context, s0)),
              [s1] "i"(offsetof(Context, s1)), [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)),
              [s4] "i"(offsetof(Context, s4)), [s5] "i"(offsetof(Context, s5)), [s6] "i"(offsetof(Context, s6)),
              [s7] "i"(offsetof(Context, s7)), [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)),
              [s10] "i"(offsetof(Context, s10)), [s11] "i"(offsetof(Context, s11)), [pc] "i"(offsetof(Context, pc)));

        asm("csrr t0, %0" ::"i"(T::SCRATCH));
        asm("sd t0, %0(%1)" ::"i"(offsetof(Context, scratch)), "r"(this));

        register uintmax_t t0 asm("t0");
        asm("csrr t0, %0" ::"i"(T::STATUS));
        t0 |= T::ME2ME;
        t0 &= ~T::PIRQE;
        asm("sd %0, %1(%2)" ::"r"(t0), "i"(offsetof(Context, status)), "r"(this));
    }

    template <bool ChangeStack = false> __attribute__((always_inline)) static inline Context *push() {

        if constexpr (ChangeStack) {
            asm volatile("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }

        asm("addi sp, sp, %[size]\n" ::[size] "i"(-sizeof(ContextHandler)));

        asm("sd ra, %[ra](sp)\n"
            "sd gp, %[gp](sp)\n"
            "sd t0, %[t0](sp)\n"
            "sd t1, %[t1](sp)\n"
            "sd t2, %[t2](sp)\n"
            "sd t3, %[t3](sp)\n"
            "sd t4, %[t4](sp)\n"
            "sd t5, %[t5](sp)\n"
            "sd t6, %[t6](sp)\n"
            "sd a0, %[a0](sp)\n"
            "sd a1, %[a1](sp)\n"
            "sd a2, %[a2](sp)\n"
            "sd a3, %[a3](sp)\n"
            "sd a4, %[a4](sp)\n"
            "sd a5, %[a5](sp)\n"
            "sd a6, %[a6](sp)\n"
            "sd a7, %[a7](sp)\n"
            :
            : [ra] "i"(offsetof(ContextHandler, ra)), [gp] "i"(offsetof(ContextHandler, gp)),
              [tp] "i"(offsetof(ContextHandler, tp)), [t0] "i"(offsetof(ContextHandler, t0)),
              [t1] "i"(offsetof(ContextHandler, t1)), [t2] "i"(offsetof(ContextHandler, t2)),
              [t3] "i"(offsetof(ContextHandler, t3)), [t4] "i"(offsetof(ContextHandler, t4)),
              [t5] "i"(offsetof(ContextHandler, t5)), [t6] "i"(offsetof(ContextHandler, t6)),
              [a0] "i"(offsetof(ContextHandler, a0)), [a1] "i"(offsetof(ContextHandler, a1)),
              [a2] "i"(offsetof(ContextHandler, a2)), [a3] "i"(offsetof(ContextHandler, a3)),
              [a4] "i"(offsetof(ContextHandler, a4)), [a5] "i"(offsetof(ContextHandler, a5)),
              [a6] "i"(offsetof(ContextHandler, a6)), [a7] "i"(offsetof(ContextHandler, a7))
            : "memory");

        asm("csrr t0, %[status]\n"
            "blt t0, zero, 1f\n"
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
            "1:"
            :
            : [status] "i"(T::STATUS), [s0] "i"(offsetof(ContextHandler, s0)), [s1] "i"(offsetof(ContextHandler, s1)),
              [s2] "i"(offsetof(ContextHandler, s2)), [s3] "i"(offsetof(ContextHandler, s3)),
              [s4] "i"(offsetof(ContextHandler, s4)), [s5] "i"(offsetof(ContextHandler, s5)),
              [s6] "i"(offsetof(ContextHandler, s6)), [s7] "i"(offsetof(ContextHandler, s7)),
              [s8] "i"(offsetof(ContextHandler, s8)), [s9] "i"(offsetof(ContextHandler, s9)),
              [s10] "i"(offsetof(ContextHandler, s10)), [s11] "i"(offsetof(ContextHandler, s11))
            : "memory");

        asm("csrr t0, %0\nsd t0, %1(sp)" ::"i"(T::STATUS), "i"(offsetof(ContextHandler, status)));
        asm("csrr t0, %0\nsd t0, %1(sp)" ::"i"(T::EPC), "i"(offsetof(ContextHandler, pc)));
        asm("csrr t0, %0\nsd t0, %1(sp)" ::"i"(T::CAUSE), "i"(offsetof(ContextHandler, cause)));
        asm("csrr t0, %0\nsd t0, %1(sp)" ::"i"(T::TVAL), "i"(offsetof(ContextHandler, tval)));

        register Context *sp asm("sp");
        return sp;
    }

    template <bool ChangeStack = false> __attribute__((naked)) static void pop() {
        asm("ld t0, %0(sp)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, status)), "i"(T::STATUS));
        asm("ld t0, %0(sp)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, pc)), "i"(T::EPC));
        asm("ld t0, %0(sp)\ncsrw %1, t0" ::"i"(offsetof(ContextHandler, cause)), "i"(T::CAUSE));

        asm("blt t0, zero, 1f\n"
            "ld s0, %[s0](sp)\n"
            "ld s1, %[s1](sp)\n"
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
            "1:" ::[s0] "i"(offsetof(ContextHandler, s0)),
            [s1] "i"(offsetof(ContextHandler, s1)), [s2] "i"(offsetof(ContextHandler, s2)),
            [s3] "i"(offsetof(ContextHandler, s3)), [s4] "i"(offsetof(ContextHandler, s4)),
            [s5] "i"(offsetof(ContextHandler, s5)), [s6] "i"(offsetof(ContextHandler, s6)),
            [s7] "i"(offsetof(ContextHandler, s7)), [s8] "i"(offsetof(ContextHandler, s8)),
            [s9] "i"(offsetof(ContextHandler, s9)), [s10] "i"(offsetof(ContextHandler, s10)),
            [s11] "i"(offsetof(ContextHandler, s11)));

        asm volatile("ld ra, %[ra](sp)\n"
                     "ld gp, %[gp](sp)\n"
                     "ld t0, %[t0](sp)\n"
                     "ld t1, %[t1](sp)\n"
                     "ld t2, %[t2](sp)\n"
                     "ld t3,  %[t3](sp)\n"
                     "ld t4,  %[t4](sp)\n"
                     "ld t5,  %[t5](sp)\n"
                     "ld t6,  %[t6](sp)\n"
                     "ld a0, %[a0](sp)\n"
                     "ld a1, %[a1](sp)\n"
                     "ld a2, %[a2](sp)\n"
                     "ld a3, %[a3](sp)\n"
                     "ld a4, %[a4](sp)\n"
                     "ld a5, %[a5](sp)\n"
                     "ld a6, %[a6](sp)\n"
                     "ld a7, %[a7](sp)\n"
                     :
                     : [ra] "i"(offsetof(ContextHandler, ra)), [gp] "i"(offsetof(ContextHandler, gp)),
                       [tp] "i"(offsetof(ContextHandler, tp)), [t0] "i"(offsetof(ContextHandler, t0)),
                       [t1] "i"(offsetof(ContextHandler, t1)), [t2] "i"(offsetof(ContextHandler, t2)),
                       [t3] "i"(offsetof(ContextHandler, t3)), [t4] "i"(offsetof(ContextHandler, t4)),
                       [t5] "i"(offsetof(ContextHandler, t5)), [t6] "i"(offsetof(ContextHandler, t6)),
                       [a0] "i"(offsetof(ContextHandler, a0)), [a1] "i"(offsetof(ContextHandler, a1)),
                       [a2] "i"(offsetof(ContextHandler, a2)), [a3] "i"(offsetof(ContextHandler, a3)),
                       [a4] "i"(offsetof(ContextHandler, a4)), [a5] "i"(offsetof(ContextHandler, a5)),
                       [a6] "i"(offsetof(ContextHandler, a6)), [a7] "i"(offsetof(ContextHandler, a7)));

        asm("addi sp, sp, %[size]\n" ::[size] "i"(sizeof(ContextHandler)));

        if constexpr (ChangeStack) {
            asm volatile("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }

        T::ret();
    }
};

using MachineContext    = ContextHandler<MachineMode>;
using SupervisorContext = ContextHandler<SupervisorMode>;

} // namespace riscv64

} // namespace DEPOS
