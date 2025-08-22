#pragma once

#include <Spin.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <cpus/riscv/clint.hpp>
#include <cpus/riscv/ic.hpp>

namespace Kernel {
    void init();
}

static inline char stack[Traits::Machine::CPUS][Traits::Memory::Page::SIZE];

struct RISCV {
    typedef uintmax_t Register;
    struct Machine {
        enum : Register {
            PMPADDR0      = 0x3B0,
            PMPCFG0       = 0x3A0,
            MHARTID       = 0xF14,
            MEDELEG       = 0x302,       // Machine Exception Delegation
            MIDELEG       = 0x303,       // Machine Interrupt Delegation
            PP            = 3 << 11,     // Previous Previlege
            ME2ME         = 3ULL << 11,  // Machine to Machine
            ME2USER       = ME2ME,       // Machine to User
            ME2SUPERVISOR = 1ULL << 11,  // Machine to Supervisor
            IRQE          = 1ULL << 3,   // Interrupt Enable
            TIE           = 1ULL << 7,   // Timer Interrupt Enable
            PIE           = 1ULL << 7,   // Previous Interrupt Enable
        };
        static constexpr const int STATUS = 0x300;
        static constexpr const int IE     = 0x304;
        static constexpr const int TVEC   = 0x305;
        static constexpr const int EPC    = 0x341;
        static constexpr const int CAUSE  = 0x342;
        static constexpr const int IP     = 0x344;
        __attribute__((always_inline)) static inline void ret() { asm volatile("mret"); }
    };

    struct Supervisor {
        enum : Register {
            PP      = 3 << 11,    // Previous Previlege
            ME2ME   = 1ULL << 8,  // Supervisor to Supervisor
            ME2USER = ME2ME,      // Supervisor to User
            IRQE    = 1ULL << 1,  // Interrupt Enable
            TIE     = 1ULL << 5,  // Timer Interrupt Enable
            PIE     = 1ULL << 5,  // Previous Interrupt Enable
        };
        static constexpr const int STATUS = 0x100;
        static constexpr const int IE     = 0x104;
        static constexpr const int TVEC   = 0x105;
        static constexpr const int EPC    = 0x141;
        static constexpr const int CAUSE  = 0x142;
        static constexpr const int IP     = 0x144;
        __attribute__((always_inline)) static inline void ret() { asm volatile("sret"); }
    };

    using CLINT = SiFiveCLINT;
    using Mode  = Supervisor;

    template <const int R>
    static void csrw(auto r) {
        asm volatile("csrw %0, %1" ::"i"(R), "r"(r));
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
        register void *gp asm("gp");
        return gp;
    }

    __attribute__((always_inline)) static inline void idle() { asm volatile("wfi"); }
    __attribute__((always_inline)) static inline void sp(void *s) { asm volatile("mv sp, %0" ::"r"(s)); }
    __attribute__((always_inline)) inline static auto core() {
        register unsigned int tp asm("tp");
        return tp;
    }

    __attribute__((naked)) static void init() {
        asm volatile("csrr tp, mhartid");
        RISCV::sp(stack[RISCV::core()] + Traits::Memory::Page::SIZE);
        // RISCV::csrc<MSTATUS>(RISCV::MACHINE_IRQ);

        if constexpr (Meta::StringCompare(Traits::Machine::NAME, "sifive_u") && Meta::SAME<Mode, Supervisor>::Result) {
            if (RISCV::core() == 0) {
                for (;;) RISCV::idle();
            }
        }

        if constexpr (Traits::Timer::Enable) {
            RISCV::csrs<Mode::IE>(static_cast<Register>(Machine::TIE) | static_cast<Register>(Supervisor::TIE));
        }

        if constexpr (Meta::SAME<Mode, Supervisor>::Result) {
            // csrw<MTVEC>(irq2s);
            // csrw<STVEC>(Kernel::ktrap);
            csrw<Machine::TVEC>(MIC::entry);
            // RISCV::csrs<Machine::IE>(Machine::TIE);
            RISCV::csrw<Machine::MEDELEG>(0xFFFF);
            RISCV::csrw<Machine::MIDELEG>(0xFFFF);
            RISCV::csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            RISCV::csrw<Machine::PMPCFG0>(0x1F);
            RISCV::csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIE);
            RISCV::csrc<Machine::STATUS>(Supervisor::PIE | Supervisor::IRQE);
        } else {
            csrs<Machine::STATUS>(Machine::ME2ME);
            csrw<Machine::TVEC>(MIC::entry);
        }

        csrw<Machine::EPC>(Kernel::init);
        Machine::ret();
    }

    struct Context {
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
            this->status = reinterpret_cast<uintptr_t>(0ULL | Mode::ME2USER | Mode::PIE);
            this->a0     = reinterpret_cast<uintptr_t>(a0);
        }

        template <typename T = Mode>
        __attribute__((naked)) static void load() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw %0, t0\n"
                "ld t0, %c[pc](sp)\n"
                "csrw %1, t0" ::"i"(T::STATUS),
                "i"(T::EPC), [pc] "i"(OFFSET_OF(Context, pc)), [status] "i"(OFFSET_OF(Context, status)));
            asm volatile(
                "ld ra, %c[ra](sp)\n"
                "ld gp, %c[gp](sp)\n"
                "ld s0, %c[s0](sp)\n"
                "ld a0, %c[a0](sp)\n"
                "ld s1, %c[s1](sp)\n"
                "ld s2, %c[s2](sp)\n"
                "ld s3, %c[s3](sp)\n"
                "ld s4, %c[s4](sp)\n"
                "ld s5, %c[s5](sp)\n"
                "ld s6, %c[s6](sp)\n"
                "ld s7, %c[s7](sp)\n"
                "ld s8, %c[s8](sp)\n"
                "ld s9, %c[s9](sp)\n"
                "ld s10, %c[s10](sp)\n"
                "ld s11, %c[s11](sp)\n"
                "addi sp, sp, %[size]\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [gp] "i"(OFFSET_OF(Context, gp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [a0] "i"(OFFSET_OF(Context, a0)), [s2] "i"(OFFSET_OF(Context, s2)),
                  [s3] "i"(OFFSET_OF(Context, s3)), [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)),
                  [s6] "i"(OFFSET_OF(Context, s6)), [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)),
                  [s9] "i"(OFFSET_OF(Context, s9)), [s10] "i"(OFFSET_OF(Context, s10)),
                  [s11] "i"(OFFSET_OF(Context, s11)), [size] "i"(sizeof(Context)));
            T::ret();
        }

        template <typename T = Mode>
        __attribute__((naked)) static void swtch(Context **previous, Context *next, Spin *lock) {
            asm volatile(
                "addi sp, sp, %[size]\n"
                "sd ra, %c[ra](sp)\n"
                "sd gp, %c[gp](sp)\n"
                "sd s0, %c[s0](sp)\n"
                "sd s1, %c[s1](sp)\n"
                "sd s2, %c[s2](sp)\n"
                "sd s3, %c[s3](sp)\n"
                "sd s4, %c[s4](sp)\n"
                "sd s5, %c[s5](sp)\n"
                "sd s6, %c[s6](sp)\n"
                "sd s7, %c[s7](sp)\n"
                "sd s8, %c[s8](sp)\n"
                "sd s9, %c[s9](sp)\n"
                "sd s10, %c[s10](sp)\n"
                "sd s11, %c[s11](sp)\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [gp] "i"(OFFSET_OF(Context, gp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [s2] "i"(OFFSET_OF(Context, s2)), [s3] "i"(OFFSET_OF(Context, s3)),
                  [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)), [s6] "i"(OFFSET_OF(Context, s6)),
                  [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)), [s9] "i"(OFFSET_OF(Context, s9)),
                  [s10] "i"(OFFSET_OF(Context, s10)), [s11] "i"(OFFSET_OF(Context, s11)), [size] "i"(-sizeof(Context))
                : "memory");
            asm volatile(
                "csrr t0, %0\n"
                "and  t0, t0, %1\n"
                "or   t0, t0, %2\n"
                "sd   t0, %c[status](sp)\n"
                "sd   ra, %c[pc](sp)\n"
                "sd   sp, (%[prev])\n"
                "mv   sp, %[next]\n"
                :
                : "i"(T::STATUS), "r"(~(T::PIE | T::PP)), "r"(T::ME2ME), [status] "i"(OFFSET_OF(Context, status)),
                  [pc] "i"(OFFSET_OF(Context, pc)), [prev] "r"(previous), [next] "r"(next)
                : "memory");
            lock->release();
            load();
        }

        template <typename T = Mode>
        __attribute__((always_inline)) static inline void push() {
            asm volatile(
                "addi sp, sp, %[size]\n"
                "sd ra, %c[ra](sp)\n"
                "sd gp, %c[gp](sp)\n"
                "sd t0, %c[t0](sp)\n"
                "sd t1, %c[t1](sp)\n"
                "sd t2, %c[t2](sp)\n"
                "sd t3, %c[t3](sp)\n"
                "sd t4, %c[t4](sp)\n"
                "sd t5, %c[t5](sp)\n"
                "sd t6, %c[t6](sp)\n"
                "sd a0, %c[a0](sp)\n"
                "sd a1, %c[a1](sp)\n"
                "sd a2, %c[a2](sp)\n"
                "sd a3, %c[a3](sp)\n"
                "sd a4, %c[a4](sp)\n"
                "sd a5, %c[a5](sp)\n"
                "sd a6, %c[a6](sp)\n"
                "sd a7, %c[a7](sp)\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [gp] "i"(OFFSET_OF(Context, gp)), [t0] "i"(OFFSET_OF(Context, t0)),
                  [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)), [t3] "i"(OFFSET_OF(Context, t3)),
                  [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)), [t6] "i"(OFFSET_OF(Context, t6)),
                  [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)), [a2] "i"(OFFSET_OF(Context, a2)),
                  [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)), [a5] "i"(OFFSET_OF(Context, a5)),
                  [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7)), [size] "i"(-sizeof(Context))
                : "memory");

            asm volatile(
                "csrr t0, %0\n"
                "sd t0, %c[status](sp)\n"
                "csrr t0, %1\n"
                "sd t0, %c[pc](sp)" ::"i"(T::STATUS),
                "i"(T::EPC), [status] "i"(OFFSET_OF(Context, status)), [pc] "i"(OFFSET_OF(Context, pc)));
        }

        template <typename T = Mode>
        __attribute__((naked)) static void pop() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw %0, t0\n"
                "ld t0, %c[pc](sp)\n"
                "csrw %1, t0" ::"i"(T::STATUS),
                "i"(T::EPC), [pc] "i"(OFFSET_OF(Context, pc)), [status] "i"(OFFSET_OF(Context, status)));
            asm volatile(
                "ld ra, %c[ra](sp)\n"
                "ld gp, %c[gp](sp)\n"
                "ld t0, %c[t0](sp)\n"
                "ld t1, %c[t1](sp)\n"
                "ld t2, %c[t2](sp)\n"
                "ld t3, %c[t3](sp)\n"
                "ld t4, %c[t4](sp)\n"
                "ld t5, %c[t5](sp)\n"
                "ld t6, %c[t6](sp)\n"
                "ld a0, %c[a0](sp)\n"
                "ld a1, %c[a1](sp)\n"
                "ld a2, %c[a2](sp)\n"
                "ld a3, %c[a3](sp)\n"
                "ld a4, %c[a4](sp)\n"
                "ld a5, %c[a5](sp)\n"
                "ld a6, %c[a6](sp)\n"
                "ld a7, %c[a7](sp)\n"
                "addi sp, sp, %[size]\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [gp] "i"(OFFSET_OF(Context, gp)), [t0] "i"(OFFSET_OF(Context, t0)),
                  [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)), [t3] "i"(OFFSET_OF(Context, t3)),
                  [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)), [t6] "i"(OFFSET_OF(Context, t6)),
                  [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)), [a2] "i"(OFFSET_OF(Context, a2)),
                  [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)), [a5] "i"(OFFSET_OF(Context, a5)),
                  [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7)), [size] "i"(sizeof(Context))
                : "memory");
            T::ret();
        }
    };

    struct Interrupt {
        static void disable() { csrc<Mode::STATUS>(Mode::IRQE); }
        static void enable() { csrs<Mode::STATUS>(Mode::IRQE); }

        static void on() { enable(); }
        static bool off() {
            Register status = csrr<Mode::STATUS>();
            disable();
            return (status & 0x8) != 0;
        }
    };
};
