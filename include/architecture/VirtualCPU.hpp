#pragma once

#include __VIRTUAL_CPU_HEADER

namespace DEPOS {

class VirtualCPU : public __ARCH::VirtualCPU {
    using __ARCH::VirtualCPU::VirtualCPU;
};

} // namespace DEPOS
