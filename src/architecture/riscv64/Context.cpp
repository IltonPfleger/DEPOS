#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/CoreContextHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VCPU.hpp>
#include <utility/Console.hpp>

__attribute__((naked)) void DEPOS::riscv64::HypervisorContext::doSwap(void *previous, void *next) {
    Father::save();
    save();
    asm("sd sp, 0(%0)" ::"r"(previous));
    asm("mv sp, %0" ::"r"(next));
    load();
    Father::load();
}

void DEPOS::riscv64::HypervisorContext::swap(void *previous, void *next) {
    HypervisorContext *ncontext = reinterpret_cast<HypervisorContext *>(next);

    VCPU *ncpu = reinterpret_cast<VCPU *>(ncontext->guest_.cpu);
    VCPU *pcpu = VCPU::current();

    if (ncpu) {
        ncpu->activate();
    } else {
        VCPU::current(nullptr);
    }

    CoreContextHandler<MachineMode>::current()->scratch0 = reinterpret_cast<uintptr_t>(pcpu);
    doSwap(previous, next);
}
