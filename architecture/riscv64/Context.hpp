#ifndef __DEPOS_RISCV64_CONTEXT__
#define __DEPOS_RISCV64_CONTEXT__

#include <Traits.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/CoreContext.hpp>
#include <architecture/riscv64/FPU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <memory/Chunk.hpp>

namespace DEPOS {

template <typename T, bool ChangeStack> class ContextTemplate : public ContextFrame {
  public:
    ContextTemplate(const Chunk &ksp, auto pc, auto ra, auto a0, auto a1) {
        this->pc     = reinterpret_cast<uint64_t>(pc);
        this->ra     = reinterpret_cast<uint64_t>(ra);
        this->status = static_cast<uint64_t>(T::ME2ME);
        this->a0     = reinterpret_cast<uint64_t>(a0);
        this->a1     = reinterpret_cast<uint64_t>(a1);
        this->ksp    = ksp.end();
    }

    static ContextTemplate *create(const Chunk &usp, const Chunk &ksp, auto pc, auto ra, auto a0, auto a1) {
        auto *context = reinterpret_cast<ContextTemplate *>(usp.end()) - 1;
        new (context) ContextTemplate(ksp, pc, ra, a0, a1);
        return context;
    }

    __attribute__((naked)) static void swap(void *previous, void *next) {
        FPU::save<T>();
        save();
        asm("sd sp, 0(%0)" ::"r"(previous));
        asm("mv sp, %0" ::"r"(next));
        load();
        FPU::load<T>();
        T::ret();
    }

    __attribute__((always_inline)) static void load() {
        if constexpr (ChangeStack) {
            asm("csrr t0, %0" ::"i"(T::SCRATCH));
            asm("ld t1, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, ksp)));
            asm("sd t1, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
        }

        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(ContextFrame, status)), "i"(T::STATUS));
        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(ContextFrame, pc)), "i"(T::EPC));

        asm("ld s0,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s0)));
        asm("ld s1,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s1)));
        asm("ld s2,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s2)));
        asm("ld s3,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s3)));
        asm("ld s4,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s4)));
        asm("ld s5,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s5)));
        asm("ld s6,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s6)));
        asm("ld s7,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s7)));
        asm("ld s8,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s8)));
        asm("ld s9,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s9)));
        asm("ld s10, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s10)));
        asm("ld s11, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s11)));

        asm("ld ra,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, ra)));
        asm("ld a1,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a1)));
        asm("ld a0,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a0)));

        asm("addi sp, sp, %0" ::"i"(sizeof(ContextFrame)));
    }

    __attribute__((always_inline)) static void save() {
        asm("addi sp, sp, %0" ::"i"(-sizeof(ContextFrame)));

        asm("sd s0,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s0)));
        asm("sd s1,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s1)));
        asm("sd s2,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s2)));
        asm("sd s3,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s3)));
        asm("sd s4,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s4)));
        asm("sd s5,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s5)));
        asm("sd s6,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s6)));
        asm("sd s7,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s7)));
        asm("sd s8,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s8)));
        asm("sd s9,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s9)));
        asm("sd s10, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s10)));
        asm("sd s11, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s11)));
        asm("sd ra,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, pc)));

        asm("csrr t0, %0" ::"i"(T::STATUS));
        asm("or   t0, t0, %0" ::"r"(T::ME2ME));
        asm("and  t0, t0, %0" ::"r"(~T::PIRQE));
        asm("sd   t0, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, status)));

        if constexpr (ChangeStack) {
            asm("csrr t0, %0" ::"i"(T::SCRATCH));
            asm("ld t1, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
            asm("sd t1, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, ksp)));
        }
    }

    __attribute__((always_inline)) static inline ContextFrame *push() {
        if constexpr (ChangeStack) {
            asm("csrrw t0, %0, t0" ::"i"(T::SCRATCH));
            asm("sd a0, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, scratch0)));
            asm("sd sp, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, scratch1)));
            asm("ld sp, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
            asm("mv a0, t0");
            asm("csrrw t0, %0, t0" ::"i"(T::SCRATCH));
        }

        asm("addi sp, sp, %0" ::"i"(-sizeof(ContextFrame)));

        asm("sd ra, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, ra)));
        asm("sd gp, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, gp)));
        asm("sd tp, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, tp)));
        asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t0)));
        asm("sd t1, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t1)));
        asm("sd t2, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t2)));
        asm("sd t3, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t3)));
        asm("sd t4, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t4)));
        asm("sd t5, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t5)));
        asm("sd t6, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, t6)));

        if constexpr (ChangeStack) {
            asm("ld t0, %0(a0)" : : "i"(__builtin_offsetof(CoreContext, scratch0)));
            asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a0)));
            asm("ld t0, %0(a0)" : : "i"(__builtin_offsetof(CoreContext, scratch1)));
            asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, sp)));
        } else {
            asm("sd a0, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a0)));
        }

        asm("sd a1, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a1)));
        asm("sd a2, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a2)));
        asm("sd a3, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a3)));
        asm("sd a4, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a4)));
        asm("sd a5, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a5)));
        asm("sd a6, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a6)));
        asm("sd a7, %0(sp)" : : "i"(__builtin_offsetof(ContextFrame, a7)));

        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::CAUSE), "i"(__builtin_offsetof(ContextFrame, cause)));
        asm("blt t0, zero, 1f");
        asm("sd s0,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s0)));
        asm("sd s1,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s1)));
        asm("sd s2,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s2)));
        asm("sd s3,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s3)));
        asm("sd s4,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s4)));
        asm("sd s5,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s5)));
        asm("sd s6,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s6)));
        asm("sd s7,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s7)));
        asm("sd s8,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s8)));
        asm("sd s9,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s9)));
        asm("sd s10, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s10)));
        asm("sd s11, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s11)));
        asm("1:");

        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::STATUS), "i"(__builtin_offsetof(ContextFrame, status)));
        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::EPC), "i"(__builtin_offsetof(ContextFrame, pc)));
        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::TVAL), "i"(__builtin_offsetof(ContextFrame, value)));

        register ContextFrame *sp asm("sp");
        return sp;
    }

    __attribute__((always_inline)) static void pop() {
        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(ContextFrame, status)), "i"(T::STATUS));
        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(ContextFrame, pc)), "i"(T::EPC));

        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, cause)));
        asm("blt t0, zero, 1f");
        asm("ld s0,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s0)));
        asm("ld s1,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s1)));
        asm("ld s2,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s2)));
        asm("ld s3,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s3)));
        asm("ld s4,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s4)));
        asm("ld s5,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s5)));
        asm("ld s6,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s6)));
        asm("ld s7,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s7)));
        asm("ld s8,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s8)));
        asm("ld s9,  %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s9)));
        asm("ld s10, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s10)));
        asm("ld s11, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, s11)));
        asm("1:");

        asm("ld ra, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, ra)));
        asm("ld gp, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, gp)));
        asm("ld tp, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, tp)));
        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t0)));
        asm("ld t1, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t1)));
        asm("ld t2, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t2)));
        asm("ld t3, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t3)));
        asm("ld t4, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t4)));
        asm("ld t5, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t5)));
        asm("ld t6, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, t6)));
        asm("ld a0, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a0)));
        asm("ld a1, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a1)));
        asm("ld a2, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a2)));
        asm("ld a3, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a3)));
        asm("ld a4, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a4)));
        asm("ld a5, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a5)));
        asm("ld a6, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a6)));
        asm("ld a7, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, a7)));

        if constexpr (ChangeStack) {
            asm("ld sp, %0(sp)" ::"i"(__builtin_offsetof(ContextFrame, sp)));
        } else {
            asm("addi sp, sp, %0" ::"i"(sizeof(ContextFrame)));
        }

        T::ret();
    }
};

template <bool ChangeStack = Traits<Thread>::IsolatedKernelStack>
using MachineContext = ContextTemplate<MachineMode, ChangeStack>;

template <bool ChangeStack = Traits<Thread>::IsolatedKernelStack>
using SupervisorContext = ContextTemplate<SupervisorMode, ChangeStack>;

struct GuestContextFrame {
    uint64_t sie      = 0;
    uint64_t stvec    = 0;
    uint64_t sscratch = 0;
    uint64_t satp     = 0;
    uint64_t sepc     = 0;
    uint64_t scause   = 0;
    uint64_t stval    = 0;
    uint64_t cpu      = 0;
};

class HypervisorContext {
    using GuestMode = SupervisorMode;
    using Father    = MachineContext<true>;

    HypervisorContext(const Chunk &ksp, auto pc, auto ra, auto a0, auto a1)
        : guest_(),
          father_(ksp, pc, ra, a0, a1) {}

  public:
    static HypervisorContext *create(const Chunk &usp, const Chunk &ksp, auto pc, auto ra, auto a0, auto a1) {
        HypervisorContext *context = reinterpret_cast<HypervisorContext *>(usp.end()) - 1;
        new (context) HypervisorContext(ksp, pc, ra, a0, a1);
        return context;
    }

    static void swap(void *previous, void *next);

  private:
    static constexpr uint32_t SIE      = __builtin_offsetof(GuestContextFrame, sie);
    static constexpr uint32_t SATP     = __builtin_offsetof(GuestContextFrame, satp);
    static constexpr uint32_t SSCRATCH = __builtin_offsetof(GuestContextFrame, sscratch);
    static constexpr uint32_t STVEC    = __builtin_offsetof(GuestContextFrame, stvec);
    static constexpr uint32_t STVAL    = __builtin_offsetof(GuestContextFrame, stval);
    static constexpr uint32_t SCAUSE   = __builtin_offsetof(GuestContextFrame, scause);
    static constexpr uint32_t SEPC     = __builtin_offsetof(GuestContextFrame, sepc);
    static constexpr uint32_t OWNER    = __builtin_offsetof(GuestContextFrame, cpu);

  private:
    __attribute__((naked)) static void doSwap(void *, void *);
    __attribute__((always_inline)) static void save();
    __attribute__((always_inline)) static void load();

  private:
    GuestContextFrame guest_;
    Father father_;
};

} // namespace DEPOS

inline void DEPOS::HypervisorContext::save() {
    asm("addi sp, sp, %0" ::"i"(-sizeof(GuestContextFrame)));
    asm("csrr t0, sscratch; sd t0, %0(sp)" ::"i"(SSCRATCH));
    asm("csrr t0, satp; sd t0, %0(sp)" ::"i"(SATP));
    asm("csrr t0, stvec; sd t0, %0(sp)" ::"i"(STVEC));
    asm("csrr t0, scause; sd t0, %0(sp)" ::"i"(SCAUSE));
    asm("csrr t0, stval; sd t0, %0(sp)" ::"i"(STVAL));
    asm("csrr t0, sepc; sd t0, %0(sp)" ::"i"(SEPC));
    asm("csrr t0, mie; sd t0, %0(sp)" ::"i"(SIE));
    asm("csrr t0, %0" ::"i"(MachineMode::SCRATCH));
    asm("ld t0, %0(t0)" : : "i"(__builtin_offsetof(CoreContext, scratch0)));
    asm("sd t0, %0(sp)" ::"i"(OWNER));
}

inline void DEPOS::HypervisorContext::load() {
    asm("ld t0, %0(sp); csrw sscratch, t0" ::"i"(SSCRATCH));
    asm("ld t0, %0(sp); csrw satp, t0" ::"i"(SATP));
    asm("ld t0, %0(sp); csrw stvec, t0" ::"i"(STVEC));
    asm("ld t0, %0(sp); csrw scause, t0" ::"i"(SCAUSE));
    asm("ld t0, %0(sp); csrw stval, t0" ::"i"(STVAL));
    asm("ld t0, %0(sp); csrw sepc, t0" ::"i"(SEPC));
    asm("li t1, 0x222; ld t0, %0(sp); andi t0, t0, 0x222; csrc mie, t1; csrs mie, t0" ::"i"(SIE));
    asm("addi sp, sp, %0" ::"i"(sizeof(GuestContextFrame)));
}

#endif
