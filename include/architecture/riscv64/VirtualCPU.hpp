#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>

namespace riscv64 {

class VirtualCPU {
  public:
    using Entry = void (*)();
    using Link = Node<VirtualCPU *>;
    using Queue = FIFO<Link>;
    using Context = ContextBase<MachineMode>;

    __attribute((naked)) VirtualCPU() {
        csrw<SupervisorMode::SATP>(0);
        PMP::NAPOT<2>(0, 0, PMP::R | PMP::W | PMP::X);
        csrw<MachineMode::MIDELEG>(0x1666);
        csrw<MachineMode::MEDELEG>(0xf4b509);
        csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
        csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
        csrw<MachineMode::EPC>(__builtin_return_address(0));
        MachineMode::ret();
    }
};

} // namespace rv
