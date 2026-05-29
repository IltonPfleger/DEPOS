#pragma once

#include __VCPU_HEADER

namespace DEPOS {

class VirtualCPU : public __ARCH::VirtualCPU {
    using __ARCH::VirtualCPU::VirtualCPU;
};

} // namespace DEPOS
