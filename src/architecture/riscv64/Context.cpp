#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/CoreContextHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
#include <utility/Console.hpp>

__attribute__((naked)) void DEPOS::HypervisorContext::doSwap(void *previous, void *next) {
    Father::save();
    save();
    asm("sd sp, 0(%0)" ::"r"(previous));
    asm("mv sp, %0" ::"r"(next));
    load();
    Father::load();
}

void DEPOS::HypervisorContext::swap(void *previous, void *next) {
    HypervisorContext *ncontext = reinterpret_cast<HypervisorContext *>(next);

    VirtualCPU *ncpu = reinterpret_cast<VirtualCPU *>(ncontext->guest_.cpu);
    VirtualCPU *pcpu = VirtualCPU::swap(ncpu);

    CoreContextHandler<MachineMode>::current()->scratch0 = reinterpret_cast<uintptr_t>(pcpu);
    doSwap(previous, next);
}
