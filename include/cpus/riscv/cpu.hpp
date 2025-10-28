#pragma once

#include <Traits.hpp>
#include <Types.hpp>
#include <cpus/riscv/clint.hpp>
#include <cpus/riscv/ic.hpp>
#include <cpus/riscv/mmu.hpp>

namespace Kernel {
    void init();
}

__attribute__((aligned(16))) static inline char stack[Traits::Machine::CPUS * Traits::Memory::Page::SIZE];

class RISCV {
   public:
    typedef uintmax_t Register;
    struct Machine {
        enum : Register {
            PMPADDR0      = 0x3B0,
            PMPCFG0       = 0x3A0,
            MHARTID       = 0xF14,       // Core Number/ID
            MEDELEG       = 0x302,       // Machine Exception Delegation
            MIDELEG       = 0x303,       // Machine Interrupt Delegation
            ME2ME         = 3ULL << 11,  // Machine to Machine
            ME2SUPERVISOR = 1ULL << 11,  // Machine to Supervisor
            TI            = 1ULL << 7,   // Timer Interrupt Enable
            IRQE          = 1ULL << 3,   // Interrupt Enable
            PIRQE         = 1ULL << 7,   // Previous Interrupt Enable
        };
        static constexpr const int STATUS = 0x300;
        static constexpr const int IE     = 0x304;
        static constexpr const int TVEC   = 0x305;
        static constexpr const int EPC    = 0x341;
        static constexpr const int CAUSE  = 0x342;
        static constexpr const int IP     = 0x344;
        static constexpr const int TVAL   = 0x343;

        __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }
        __attribute__((always_inline)) static inline void ecall() { asm volatile("ecall"); }
    };

    struct Supervisor {
        enum : Register {
            ME2ME = 1ULL << 8,  // Supervisor to Supervisor
            IRQE  = 1ULL << 1,  // Interrupt Enable
            TI    = 1ULL << 5,  // Timer Interrupt Enable
            PIRQE = 1ULL << 5,  // Previous Interrupt Enable
        };
        static constexpr const int SATP   = 0x180;
        static constexpr const int STATUS = 0x100;
        static constexpr const int IE     = 0x104;
        static constexpr const int TVEC   = 0x105;
        static constexpr const int EPC    = 0x141;
        static constexpr const int CAUSE  = 0x142;
        static constexpr const int IP     = 0x144;
        __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }
    };

    using CLINT = SiFiveCLINT;
    using MMU   = SV39_MMU;
    using Mode  = Supervisor;

    template <const int R>
    static void csrw(auto r) {
        asm volatile("csrw %c0, %1" ::"i"(R), "r"(r));
    }

    template <const int R>
    static auto csrr() {
        Register r;
        asm volatile("csrr %0, %1" : "=r"(r) : "i"(R));
        return r;
    }

    template <const int R>
    static void csrs(auto r) {
        asm volatile("csrs %0, %1" ::"i"(R), "r"(r));
    }

    template <const int R>
    static void csrc(auto r) {
        asm volatile("csrc %0, %1" ::"i"(R), "r"(r));
    }

    static void *thread() {
        void *gp;
        asm volatile("mv %0, gp" : "=r"(gp));
        return gp;
    }

    __attribute__((always_inline)) static inline void idle() { asm volatile("wfi"); }

    inline static auto core() {
        register unsigned int tp asm("tp");
        return tp;
    }

    __attribute__((naked)) static void setup() {
        __asm__ volatile(
            "csrr tp, mhartid\n"
            "csrr t0, mhartid\n"
            "mul t0, t0, %1\n"
            "add t0, t0, %0\n"
            "add sp, t0, %1\n"
            "ret"
            :
            : "r"(stack), "r"(Traits::Memory::Page::SIZE));
    }

    static void init() {
        if constexpr (Meta::StringCompare(Traits::Machine::NAME, "sifive_u") && Meta::SAME<Mode, Supervisor>::Result) {
            if (core() == 0) {
                for (;;) idle();
            }
        }

        if constexpr (Traits::Timer::Enable) {
            csrs<Machine::IE>(Machine::TI);
            csrs<Machine::IE>(Supervisor::TI);
        }

        if constexpr (Meta::SAME<Mode, Supervisor>::Result) {
            csrw<Supervisor::TVEC>(SIC::entry);
            csrw<Machine::TVEC>(MIC::entry);
            csrw<Machine::MIDELEG>(0x222);
            csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<Machine::PMPCFG0>(0b11111);
            csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIRQE);
            csrc<Machine::STATUS>(Supervisor::PIRQE | Supervisor::IRQE);
        } else {
            csrs<Machine::STATUS>(Machine::ME2ME);
            csrw<Machine::TVEC>(MIC::entry);
        }

        csrw<Machine::EPC>(Kernel::init);
        Machine::ret();
    }

    class Context {
       public:
        uintptr_t ra;
        uintptr_t gp;
        uintptr_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uintptr_t a0, a1, a2, a3, a4, a5, a6, a7;
        uintptr_t t0, t1, t2, t3, t4, t5, t6;
        uintptr_t status;
        uintptr_t pc;

        Context(int (*entry)(void *), void *a0, void (*exit)(), void *gp) {
            this->ra     = reinterpret_cast<uintptr_t>(exit);
            this->gp     = reinterpret_cast<uintptr_t>(gp);
            this->pc     = reinterpret_cast<uintptr_t>(entry);
            this->status = static_cast<Register>(Mode::ME2ME | Mode::PIRQE);
            this->a0     = reinterpret_cast<uintptr_t>(a0);
        }

        template <typename T = Mode>
        static void load(Context **c) {
            asm volatile(
                "ld sp, (%[c])\n"
                "sd zero, (%[c])\n"
                "ld ra,	 %[ra](sp)\n"
                "ld gp,  %[gp](sp)\n"
                "ld s0,  %[s0](sp)\n"
                "ld a0,  %[a0](sp)\n"
                "ld s1,  %[s1](sp)\n"
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
                "ld t0,  %[status](sp)\n"
                "ld t1,  %[pc](sp)\n"
                "csrw    %[estatus], t0\n"
                "csrw    %[epc], t1\n"
                "addi sp, sp, %[size]\n"
                :
                : [c] "r"(c), [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)),
                  [s0] "i"(offsetof(Context, s0)), [s1] "i"(offsetof(Context, s1)), [a0] "i"(offsetof(Context, a0)),
                  [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)), [s4] "i"(offsetof(Context, s4)),
                  [s5] "i"(offsetof(Context, s5)), [s6] "i"(offsetof(Context, s6)), [s7] "i"(offsetof(Context, s7)),
                  [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)), [s10] "i"(offsetof(Context, s10)),
                  [s11] "i"(offsetof(Context, s11)), [size] "i"(sizeof(Context)), [estatus] "i"(T::STATUS),
                  [epc] "i"(T::EPC), [pc] "i"(offsetof(Context, pc)), [status] "i"(offsetof(Context, status)));
            T::ret();
            __builtin_unreachable();
        }

        template <typename T = Mode>
        __attribute__((naked)) static void swtch(Context **previous, Context **next) {
            asm volatile(
                "addi sp, sp, %[size]\n"
                "sd   ra,     %[ra](sp)\n"
                "sd   gp,     %[gp](sp)\n"
                "sd   s0,     %[s0](sp)\n"
                "sd   s1,     %[s1](sp)\n"
                "sd   s2,     %[s2](sp)\n"
                "sd   s3,     %[s3](sp)\n"
                "sd   s4,     %[s4](sp)\n"
                "sd   s5,     %[s5](sp)\n"
                "sd   s6,     %[s6](sp)\n"
                "sd   s7,     %[s7](sp)\n"
                "sd   s8,     %[s8](sp)\n"
                "sd   s9,     %[s9](sp)\n"
                "sd   s10,    %[s10](sp)\n"
                "sd   s11,    %[s11](sp)\n"
                "csrr t0,     %[estatus]\n"
                "or  t0, t0, %[me2me]\n"
                "sd   t0,     %[status](sp)\n"
                "sd   ra,     %[pc](sp)\n"
                "sd   sp,    (%[previous])\n"
                :
                : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [s0] "i"(offsetof(Context, s0)),
                  [s1] "i"(offsetof(Context, s1)), [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)),
                  [s4] "i"(offsetof(Context, s4)), [s5] "i"(offsetof(Context, s5)), [s6] "i"(offsetof(Context, s6)),
                  [s7] "i"(offsetof(Context, s7)), [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)),
                  [s10] "i"(offsetof(Context, s10)), [s11] "i"(offsetof(Context, s11)), [size] "i"(-sizeof(Context)),
                  [estatus] "i"(T::STATUS), [me2me] "r"(T::ME2ME), [status] "i"(offsetof(Context, status)),
                  [pc] "i"(offsetof(Context, pc)), [previous] "r"(previous)
                : "memory");
            load(next);
        }

        template <typename T = Mode>
        __attribute__((always_inline)) static inline Context *push() {
            asm volatile(
                "addi sp, sp, %[size]\n"
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
                : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [t0] "i"(offsetof(Context, t0)),
                  [t1] "i"(offsetof(Context, t1)), [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
                  [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)), [t6] "i"(offsetof(Context, t6)),
                  [a0] "i"(offsetof(Context, a0)), [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
                  [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)), [a5] "i"(offsetof(Context, a5)),
                  [a6] "i"(offsetof(Context, a6)), [a7] "i"(offsetof(Context, a7)), [size] "i"(-sizeof(Context))
                : "memory");

            asm volatile(
                "csrr t0, %0\n"
                "sd t0, %c[status](sp)\n"
                "csrr t0, %1\n"
                "sd t0, %c[pc](sp)" ::"i"(T::STATUS),
                "i"(T::EPC), [status] "i"(offsetof(Context, status)), [pc] "i"(offsetof(Context, pc)));
            register Context *sp asm("sp");
            return sp;
        }

        template <typename T = Mode>
        __attribute__((naked)) static void pop() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw %0, t0\n"
                "ld t0, %c[pc](sp)\n"
                "csrw %1, t0" ::"i"(T::STATUS),
                "i"(T::EPC), [pc] "i"(offsetof(Context, pc)), [status] "i"(offsetof(Context, status)));
            asm volatile(
                "ld ra, %[ra](sp)\n"
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
                : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)), [t0] "i"(offsetof(Context, t0)),
                  [t1] "i"(offsetof(Context, t1)), [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
                  [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)), [t6] "i"(offsetof(Context, t6)),
                  [a0] "i"(offsetof(Context, a0)), [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
                  [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)), [a5] "i"(offsetof(Context, a5)),
                  [a6] "i"(offsetof(Context, a6)), [a7] "i"(offsetof(Context, a7)), [size] "i"(sizeof(Context))
                : "memory");
            T::ret();
            __builtin_unreachable();
        }
    };

    struct Interrupt {
        static void disable() { csrc<Mode::STATUS>(Mode::IRQE); }
        static void enable() { csrs<Mode::STATUS>(Mode::IRQE); }

        static void on() { enable(); }
        static bool off() {
            Register status = csrr<Mode::STATUS>();
            disable();
            return (status & Mode::IRQE) != 0;
        }
    };
};
