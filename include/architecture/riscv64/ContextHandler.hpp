#ifndef __RISCV64_CONTEXT_HANDLER__
#define __RISCV64_CONTEXT_HANDLER__

#include <Traits.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/CoreContext.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <shared/memory/Chunk.hpp>

namespace DEPOS {

namespace riscv64 {

template <typename T, bool ChangeStack> class ContextHandler : public Context {
  public:
    static ContextHandler *create(const Chunk &usp, const Chunk &ksp, auto pc, auto a0, auto a1) {
        ContextHandler *context = reinterpret_cast<ContextHandler *>(usp.end()) - 1;
        context->pc             = reinterpret_cast<uint64_t>(pc);
        context->status         = static_cast<uint64_t>(T::ME2ME);
        context->a0             = reinterpret_cast<uint64_t>(a0);
        context->a1             = reinterpret_cast<uint64_t>(a1);
        context->ksp            = ksp.end();
        return context;
    }

    __attribute__((naked)) static void swap(void *previous, void *next) {
        save();
        asm("sd sp, 0(%0)" ::"r"(previous));
        asm("mv sp, %0" ::"r"(next));
        load();
    }

    __attribute__((naked)) static void load() {
        if constexpr (ChangeStack) {
            // asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(Context, ksp)));
            // asm("csrw %0, t0" ::"i"(T::SCRATCH));
            asm("csrr t0, %0" ::"i"(T::SCRATCH));
            asm("ld t1, %0(sp)" ::"i"(__builtin_offsetof(Context, ksp)));
            asm("sd t1, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
        }

        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(ContextHandler, status)));
        asm("csrw %0, t0" ::"i"(T::STATUS));

        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(ContextHandler, pc)));
        asm("csrw %0, t0" ::"i"(T::EPC));

        asm("ld s0,  %0(sp)" ::"i"(__builtin_offsetof(Context, s0)));
        asm("ld s1,  %0(sp)" ::"i"(__builtin_offsetof(Context, s1)));
        asm("ld s2,  %0(sp)" ::"i"(__builtin_offsetof(Context, s2)));
        asm("ld s3,  %0(sp)" ::"i"(__builtin_offsetof(Context, s3)));
        asm("ld s4,  %0(sp)" ::"i"(__builtin_offsetof(Context, s4)));
        asm("ld s5,  %0(sp)" ::"i"(__builtin_offsetof(Context, s5)));
        asm("ld s6,  %0(sp)" ::"i"(__builtin_offsetof(Context, s6)));
        asm("ld s7,  %0(sp)" ::"i"(__builtin_offsetof(Context, s7)));
        asm("ld s8,  %0(sp)" ::"i"(__builtin_offsetof(Context, s8)));
        asm("ld s9,  %0(sp)" ::"i"(__builtin_offsetof(Context, s9)));
        asm("ld s10, %0(sp)" ::"i"(__builtin_offsetof(Context, s10)));
        asm("ld s11, %0(sp)" ::"i"(__builtin_offsetof(Context, s11)));

        asm("ld a1,  %0(sp)" ::"i"(__builtin_offsetof(Context, a1)));
        asm("ld a0,  %0(sp)" ::"i"(__builtin_offsetof(Context, a0)));

        asm("addi sp, sp, %0" ::"i"(sizeof(ContextHandler)));

        T::ret();
    }

    __attribute__((always_inline)) static void save() {
        asm("addi sp, sp, %0" ::"i"(-sizeof(ContextHandler)));

        asm("sd s0,  %0(sp)" ::"i"(__builtin_offsetof(Context, s0)));
        asm("sd s1,  %0(sp)" ::"i"(__builtin_offsetof(Context, s1)));
        asm("sd s2,  %0(sp)" ::"i"(__builtin_offsetof(Context, s2)));
        asm("sd s3,  %0(sp)" ::"i"(__builtin_offsetof(Context, s3)));
        asm("sd s4,  %0(sp)" ::"i"(__builtin_offsetof(Context, s4)));
        asm("sd s5,  %0(sp)" ::"i"(__builtin_offsetof(Context, s5)));
        asm("sd s6,  %0(sp)" ::"i"(__builtin_offsetof(Context, s6)));
        asm("sd s7,  %0(sp)" ::"i"(__builtin_offsetof(Context, s7)));
        asm("sd s8,  %0(sp)" ::"i"(__builtin_offsetof(Context, s8)));
        asm("sd s9,  %0(sp)" ::"i"(__builtin_offsetof(Context, s9)));
        asm("sd s10, %0(sp)" ::"i"(__builtin_offsetof(Context, s10)));
        asm("sd s11, %0(sp)" ::"i"(__builtin_offsetof(Context, s11)));
        asm("sd ra,  %0(sp)" ::"i"(__builtin_offsetof(Context, pc)));

        asm("csrr t0, %0" ::"i"(T::STATUS));
        asm("or   t0, t0, %0" ::"r"(T::ME2ME));
        asm("and  t0, t0, %0" ::"r"(~T::PIRQE));
        asm("sd   t0, %0(sp)" ::"i"(__builtin_offsetof(Context, status)));

        if constexpr (ChangeStack) {
            asm("csrr t0, %0" ::"i"(T::SCRATCH));
            asm("ld t1, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
            asm("sd t1, %0(sp)" ::"i"(__builtin_offsetof(Context, ksp)));
        }
    }

    __attribute__((always_inline)) static inline Context *push() {
        if constexpr (ChangeStack) {
            asm("csrrw t0, %0, t0" ::"i"(T::SCRATCH));
            asm("sd a0, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, scratch0)));
            asm("sd sp, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, scratch1)));
            asm("ld sp, %0(t0)" ::"i"(__builtin_offsetof(CoreContext, ksp)));
            asm("mv a0, t0");
            asm("csrrw t0, %0, t0" ::"i"(T::SCRATCH));
        }

        asm("addi sp, sp, %[size]\n" ::[size] "i"(-sizeof(ContextHandler)));

        asm("sd ra, %0(sp)" : : "i"(__builtin_offsetof(Context, ra)));
        asm("sd gp, %0(sp)" : : "i"(__builtin_offsetof(Context, gp)));
        asm("sd tp, %0(sp)" : : "i"(__builtin_offsetof(Context, tp)));
        asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(Context, t0)));
        asm("sd t1, %0(sp)" : : "i"(__builtin_offsetof(Context, t1)));
        asm("sd t2, %0(sp)" : : "i"(__builtin_offsetof(Context, t2)));
        asm("sd t3, %0(sp)" : : "i"(__builtin_offsetof(Context, t3)));
        asm("sd t4, %0(sp)" : : "i"(__builtin_offsetof(Context, t4)));
        asm("sd t5, %0(sp)" : : "i"(__builtin_offsetof(Context, t5)));
        asm("sd t6, %0(sp)" : : "i"(__builtin_offsetof(Context, t6)));

        if constexpr (ChangeStack) {
            asm("ld t0, %0(a0)" : : "i"(__builtin_offsetof(CoreContext, scratch0)));
            asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(Context, a0)));
            asm("ld t0, %0(a0)" : : "i"(__builtin_offsetof(CoreContext, scratch1)));
            asm("sd t0, %0(sp)" : : "i"(__builtin_offsetof(Context, sp)));
        } else {
            asm("sd a0, %0(sp)" : : "i"(__builtin_offsetof(Context, a0)));
        }

        asm("sd a1, %0(sp)" : : "i"(__builtin_offsetof(Context, a1)));
        asm("sd a2, %0(sp)" : : "i"(__builtin_offsetof(Context, a2)));
        asm("sd a3, %0(sp)" : : "i"(__builtin_offsetof(Context, a3)));
        asm("sd a4, %0(sp)" : : "i"(__builtin_offsetof(Context, a4)));
        asm("sd a5, %0(sp)" : : "i"(__builtin_offsetof(Context, a5)));
        asm("sd a6, %0(sp)" : : "i"(__builtin_offsetof(Context, a6)));
        asm("sd a7, %0(sp)" : : "i"(__builtin_offsetof(Context, a7)));

        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::CAUSE), "i"(__builtin_offsetof(Context, cause)));
        asm("blt t0, zero, 1f");
        asm("sd s0,  %0(sp)" ::"i"(__builtin_offsetof(Context, s0)));
        asm("sd s1,  %0(sp)" ::"i"(__builtin_offsetof(Context, s1)));
        asm("sd s2,  %0(sp)" ::"i"(__builtin_offsetof(Context, s2)));
        asm("sd s3,  %0(sp)" ::"i"(__builtin_offsetof(Context, s3)));
        asm("sd s4,  %0(sp)" ::"i"(__builtin_offsetof(Context, s4)));
        asm("sd s5,  %0(sp)" ::"i"(__builtin_offsetof(Context, s5)));
        asm("sd s6,  %0(sp)" ::"i"(__builtin_offsetof(Context, s6)));
        asm("sd s7,  %0(sp)" ::"i"(__builtin_offsetof(Context, s7)));
        asm("sd s8,  %0(sp)" ::"i"(__builtin_offsetof(Context, s8)));
        asm("sd s9,  %0(sp)" ::"i"(__builtin_offsetof(Context, s9)));
        asm("sd s10, %0(sp)" ::"i"(__builtin_offsetof(Context, s10)));
        asm("sd s11, %0(sp)" ::"i"(__builtin_offsetof(Context, s11)));
        asm("1:");

        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::STATUS), "i"(__builtin_offsetof(Context, status)));
        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::EPC), "i"(__builtin_offsetof(Context, pc)));
        asm("csrr t0, %0; sd t0, %1(sp)" ::"i"(T::TVAL), "i"(__builtin_offsetof(Context, value)));

        register Context *sp asm("sp");
        return sp;
    }

    __attribute__((naked)) static void pop() {
        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(Context, status)), "i"(T::STATUS));
        asm("ld t0, %0(sp); csrw %1, t0" ::"i"(__builtin_offsetof(Context, pc)), "i"(T::EPC));

        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(Context, cause)));
        asm("blt t0, zero, 1f");
        asm("ld s0,  %0(sp)" ::"i"(__builtin_offsetof(Context, s0)));
        asm("ld s1,  %0(sp)" ::"i"(__builtin_offsetof(Context, s1)));
        asm("ld s2,  %0(sp)" ::"i"(__builtin_offsetof(Context, s2)));
        asm("ld s3,  %0(sp)" ::"i"(__builtin_offsetof(Context, s3)));
        asm("ld s4,  %0(sp)" ::"i"(__builtin_offsetof(Context, s4)));
        asm("ld s5,  %0(sp)" ::"i"(__builtin_offsetof(Context, s5)));
        asm("ld s6,  %0(sp)" ::"i"(__builtin_offsetof(Context, s6)));
        asm("ld s7,  %0(sp)" ::"i"(__builtin_offsetof(Context, s7)));
        asm("ld s8,  %0(sp)" ::"i"(__builtin_offsetof(Context, s8)));
        asm("ld s9,  %0(sp)" ::"i"(__builtin_offsetof(Context, s9)));
        asm("ld s10, %0(sp)" ::"i"(__builtin_offsetof(Context, s10)));
        asm("ld s11, %0(sp)" ::"i"(__builtin_offsetof(Context, s11)));
        asm("1:");

        asm("ld ra, %0(sp)" ::"i"(__builtin_offsetof(Context, ra)));
        asm("ld gp, %0(sp)" ::"i"(__builtin_offsetof(Context, gp)));
        asm("ld tp, %0(sp)" ::"i"(__builtin_offsetof(Context, tp)));
        asm("ld t0, %0(sp)" ::"i"(__builtin_offsetof(Context, t0)));
        asm("ld t1, %0(sp)" ::"i"(__builtin_offsetof(Context, t1)));
        asm("ld t2, %0(sp)" ::"i"(__builtin_offsetof(Context, t2)));
        asm("ld t3, %0(sp)" ::"i"(__builtin_offsetof(Context, t3)));
        asm("ld t4, %0(sp)" ::"i"(__builtin_offsetof(Context, t4)));
        asm("ld t5, %0(sp)" ::"i"(__builtin_offsetof(Context, t5)));
        asm("ld t6, %0(sp)" ::"i"(__builtin_offsetof(Context, t6)));
        asm("ld a0, %0(sp)" ::"i"(__builtin_offsetof(Context, a0)));
        asm("ld a1, %0(sp)" ::"i"(__builtin_offsetof(Context, a1)));
        asm("ld a2, %0(sp)" ::"i"(__builtin_offsetof(Context, a2)));
        asm("ld a3, %0(sp)" ::"i"(__builtin_offsetof(Context, a3)));
        asm("ld a4, %0(sp)" ::"i"(__builtin_offsetof(Context, a4)));
        asm("ld a5, %0(sp)" ::"i"(__builtin_offsetof(Context, a5)));
        asm("ld a6, %0(sp)" ::"i"(__builtin_offsetof(Context, a6)));
        asm("ld a7, %0(sp)" ::"i"(__builtin_offsetof(Context, a7)));

        if constexpr (ChangeStack) {
            asm("ld sp, %0(sp)" ::"i"(__builtin_offsetof(Context, sp)));
        } else {
            asm("addi sp, sp, %0" ::"i"(sizeof(ContextHandler)));
        }

        T::ret();
    }
};

class HypervisorContext;

template <bool ChangeStack = Traits<Thread>::IsolatedKernelStack>
using MachineContext = ContextHandler<MachineMode, ChangeStack>;

template <bool ChangeStack = Traits<Thread>::IsolatedKernelStack>
using SupervisorContext = ContextHandler<SupervisorMode, ChangeStack>;

class HypervisorContext {
    using HostMode  = MachineMode;
    using GuestMode = SupervisorMode;
    using Host      = MachineContext<true>;

    struct GuestContext {
        uint64_t m_satp;
        uint64_t m_sscratch;
    };

  public:
    static HypervisorContext *create(const Chunk &usp, const Chunk &ksp, auto pc, auto a0, auto a1) {
        Host *host          = Host::create(usp, ksp, pc, a0, a1);
        GuestContext *guest = reinterpret_cast<GuestContext *>(host) - 1;
        guest->m_satp       = 0;
        return reinterpret_cast<HypervisorContext *>(guest);
    }

  public:
    __attribute__((naked)) static void swap(void *previous, void *next) {
        Host::save();

        asm("addi sp, sp, %0" ::"i"(-sizeof(GuestContext)));

        asm("csrr t0, %0\nsd t0, %0(sp)" ::"i"(GuestMode::SATP), "i"(__builtin_offsetof(GuestContext, m_satp)));
        asm("csrr t0, %0\nsd t0, %0(sp)" ::"i"(GuestMode::SCRATCH), "i"(__builtin_offsetof(GuestContext, m_sscratch)));

        asm("sd sp, 0(%0)" ::"r"(previous));
        asm("mv sp, %0" ::"r"(next));

        asm("ld t0, %0(sp)\ncsrw %1, t0" ::"i"(__builtin_offsetof(GuestContext, m_satp)), "i"(GuestMode::SATP));
        asm("ld t0, %0(sp)\ncsrw %1, t0" ::"i"(__builtin_offsetof(GuestContext, m_sscratch)), "i"(GuestMode::SCRATCH));

        asm("addi sp, sp, %0" ::"i"(sizeof(GuestContext)));

        Host::load();
    }
};

} // namespace riscv64

} // namespace DEPOS

#endif
