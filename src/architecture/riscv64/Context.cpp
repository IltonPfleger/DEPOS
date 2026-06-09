#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/CoreContextHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <utility/Console.hpp>

void DEPOS::HypervisorContext::swap(HypervisorContext &previous, HypervisorContext &next) {
    VirtualCPU *ncpu = reinterpret_cast<VirtualCPU *>(next.guest_.vcpu);
    VirtualCPU *pcpu = VirtualCPU::swap(ncpu);

    if (pcpu) {
        previous.guest_.vcpu = reinterpret_cast<uintptr_t>(pcpu);
        previous.save();
    }

    if (next.guest_.vcpu) {
        next.load();
    }

    Father::swap(previous.father_, next.father_);
}

void DEPOS::HypervisorContext::save() {
    guest_.sscratch = csrr<SupervisorMode::SCRATCH>();
    guest_.satp     = csrr<SupervisorMode::SATP>();
    guest_.stvec    = csrr<SupervisorMode::TVEC>();
    guest_.scause   = csrr<SupervisorMode::CAUSE>();
    guest_.stval    = csrr<SupervisorMode::TVAL>();
    guest_.sepc     = csrr<SupervisorMode::EPC>();
    guest_.sie      = csrr<MachineMode::IE>();
}

void DEPOS::HypervisorContext::load() {
    csrw<SupervisorMode::SCRATCH>(guest_.sscratch);
    csrw<SupervisorMode::SATP>(guest_.satp);
    csrw<SupervisorMode::TVEC>(guest_.stvec);
    csrw<SupervisorMode::CAUSE>(guest_.scause);
    csrw<SupervisorMode::TVAL>(guest_.stval);
    csrw<SupervisorMode::EPC>(guest_.sepc);
    csrc<MachineMode::IE>(0x222);
    csrs<MachineMode::IE>(guest_.sie & 0x222);
}
