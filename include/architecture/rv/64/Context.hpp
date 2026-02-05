#pragma once

#include <utils/string.hpp>

namespace rv64 {
template <typename T> class ContextBase {
  public:
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
    uint64_t status, pc;

    uint64_t &operator[](size_t i) { return (reinterpret_cast<uint64_t *>(&ra))[i - 1]; }

    ContextBase() = default;

    ContextBase(int (*entry)(void *), void *a0, void *sp, void (*exit)()) {
        this->ra = reinterpret_cast<uint64_t>(exit);
        this->pc = reinterpret_cast<uint64_t>(entry);
        this->status = static_cast<uint64_t>(T::ME2ME | T::PIRQE);
        this->a0 = reinterpret_cast<uint64_t>(a0);
        this->sp = reinterpret_cast<uint64_t>(sp);
    }

    template <typename F, typename... Args> __attribute__((naked)) void load(F f, Args... args) {
        asm("ld sp, %[sp](a0)" ::[sp] "i"(offsetof(ContextBase, sp)));
        asm("mv s0, a0");
        asm("addi sp, sp, %0" ::"i"(-sizeof(ContextBase)));
        f(args...);
        asm("addi sp, sp, %0" ::"i"(sizeof(ContextBase)));
        asm("mv a1, s0");
        asm("ld ra, %[ra](a1)\n"
            "ld gp, %[gp](a1)\n"
            "ld s0, %[s0](a1)\n"
            "ld a0, %[a0](a1)\n"
            "ld s1, %[s1](a1)\n"
            "ld s2, %[s2](a1)\n"
            "ld s3, %[s3](a1)\n"
            "ld s4, %[s4](a1)\n"
            "ld s5, %[s5](a1)\n"
            "ld s6, %[s6](a1)\n"
            "ld s7, %[s7](a1)\n"
            "ld s8, %[s8](a1)\n"
            "ld s9, %[s9](a1)\n"
            "ld s10, %[s10](a1)\n"
            "ld s11, %[s11](a1)\n"
            "ld t0, %[pc](a1)\n"
            "ld t1, %[status](a1)\n"
            "csrw %[csrepc], t0\n"
            "csrw %[csrstatus], t1\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [s0] "i"(offsetof(ContextBase, s0)),
              [s1] "i"(offsetof(ContextBase, s1)), [a0] "i"(offsetof(ContextBase, a0)), [s2] "i"(offsetof(ContextBase, s2)),
              [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)), [s5] "i"(offsetof(ContextBase, s5)),
              [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)), [s8] "i"(offsetof(ContextBase, s8)),
              [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)), [s11] "i"(offsetof(ContextBase, s11)),
              [pc] "i"(offsetof(ContextBase, pc)), [status] "i"(offsetof(ContextBase, status)), [csrepc] "i"(T::EPC),
              [csrstatus] "i"(T::STATUS));
        T::ret();
    }

    __attribute__((naked)) bool save() {
        asm("sd ra, %[ra](a0)\n"
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
            "csrr t1, %[csrstatus]\n"
            "or t1, t1, %[me2me]\n"
            "sd t1, %[status](a0)\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [sp] "i"(offsetof(ContextBase, sp)),
              [a0] "i"(offsetof(ContextBase, a0)), [s0] "i"(offsetof(ContextBase, s0)), [s1] "i"(offsetof(ContextBase, s1)),
              [s2] "i"(offsetof(ContextBase, s2)), [s3] "i"(offsetof(ContextBase, s3)), [s4] "i"(offsetof(ContextBase, s4)),
              [s5] "i"(offsetof(ContextBase, s5)), [s6] "i"(offsetof(ContextBase, s6)), [s7] "i"(offsetof(ContextBase, s7)),
              [s8] "i"(offsetof(ContextBase, s8)), [s9] "i"(offsetof(ContextBase, s9)), [s10] "i"(offsetof(ContextBase, s10)),
              [s11] "i"(offsetof(ContextBase, s11)), [pc] "i"(offsetof(ContextBase, pc)), [csrstatus] "i"(T::STATUS),
              [me2me] "r"(T::ME2ME), [status] "i"(offsetof(ContextBase, status))
            : "memory");
        asm("li a0, 1\n"
            "ret\n");
    }

    template <bool ChangeStack = false> __attribute__((always_inline)) static inline ContextBase *push() {
        if constexpr (ChangeStack) {
            asm("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }
        asm("addi sp, sp, %[size]\n"
            "sd ra, %[ra](sp)\n"
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
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [t0] "i"(offsetof(ContextBase, t0)),
              [t1] "i"(offsetof(ContextBase, t1)), [t2] "i"(offsetof(ContextBase, t2)), [t3] "i"(offsetof(ContextBase, t3)),
              [t4] "i"(offsetof(ContextBase, t4)), [t5] "i"(offsetof(ContextBase, t5)), [t6] "i"(offsetof(ContextBase, t6)),
              [a0] "i"(offsetof(ContextBase, a0)), [a1] "i"(offsetof(ContextBase, a1)), [a2] "i"(offsetof(ContextBase, a2)),
              [a3] "i"(offsetof(ContextBase, a3)), [a4] "i"(offsetof(ContextBase, a4)), [a5] "i"(offsetof(ContextBase, a5)),
              [a6] "i"(offsetof(ContextBase, a6)), [a7] "i"(offsetof(ContextBase, a7)), [size] "i"(-sizeof(ContextBase))
            : "memory");

        asm("csrr t0, %0\n"
            "sd t0, %c[status](sp)\n"
            "csrr t0, %1\n"
            "sd t0, %c[pc](sp)" ::"i"(T::STATUS),
            "i"(T::EPC), [status] "i"(offsetof(ContextBase, status)), [pc] "i"(offsetof(ContextBase, pc)));
        register ContextBase *sp asm("sp");
        return sp;
    }

    template <bool ChangeStack = false> __attribute__((naked)) static void pop() {
        asm("ld t0, %[statuso](sp)\n"
            "csrw %[statusr], t0\n"
            "ld t0, %[pc](sp)\n"
            "csrw %[epcr], t0" ::[statusr] "i"(T::STATUS),
            [epcr] "i"(T::EPC), [pc] "i"(offsetof(ContextBase, pc)), [statuso] "i"(offsetof(ContextBase, status)));
        asm("ld ra, %[ra](sp)\n"
            "ld gp, %[gp](sp)\n"
            "ld t0, %[t0](sp)\n"
            "ld t1, %[t1](sp)\n"
            "ld t2, %[t2](sp)\n"
            "ld t3, %[t3](sp)\n"
            "ld t4, %[t4](sp)\n"
            "ld t5, %[t5](sp)\n"
            "ld t6, %[t6](sp)\n"
            "ld a0, %[a0](sp)\n"
            "ld a1, %[a1](sp)\n"
            "ld a2, %[a2](sp)\n"
            "ld a3, %[a3](sp)\n"
            "ld a4, %[a4](sp)\n"
            "ld a5, %[a5](sp)\n"
            "ld a6, %[a6](sp)\n"
            "ld a7, %[a7](sp)\n"
            "addi sp, sp, %[size]\n"
            :
            : [ra] "i"(offsetof(ContextBase, ra)), [gp] "i"(offsetof(ContextBase, gp)), [t0] "i"(offsetof(ContextBase, t0)),
              [t1] "i"(offsetof(ContextBase, t1)), [t2] "i"(offsetof(ContextBase, t2)), [t3] "i"(offsetof(ContextBase, t3)),
              [t4] "i"(offsetof(ContextBase, t4)), [t5] "i"(offsetof(ContextBase, t5)), [t6] "i"(offsetof(ContextBase, t6)),
              [a0] "i"(offsetof(ContextBase, a0)), [a1] "i"(offsetof(ContextBase, a1)), [a2] "i"(offsetof(ContextBase, a2)),
              [a3] "i"(offsetof(ContextBase, a3)), [a4] "i"(offsetof(ContextBase, a4)), [a5] "i"(offsetof(ContextBase, a5)),
              [a6] "i"(offsetof(ContextBase, a6)), [a7] "i"(offsetof(ContextBase, a7)), [size] "i"(sizeof(ContextBase))
            : "memory");
        if constexpr (ChangeStack) {
            asm("csrrw sp, %0, sp" ::"i"(T::SCRATCH));
        }
        T::ret();
    }
};
} // namespace rv64
