#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/CoreContextHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <utility/Console.hpp>

void DEPOS::HypervisorContext::swap(HypervisorContext &previous, HypervisorContext &next) {
    VirtualCPU *ncpu = reinterpret_cast<VirtualCPU *>(next.vcpu_);
    VirtualCPU *pcpu = VirtualCPU::current();
    previous.vcpu_   = pcpu;

    if (pcpu) pcpu->save();
    if (ncpu) ncpu->activate();

    Father::swap(previous.father_, next.father_);
}
