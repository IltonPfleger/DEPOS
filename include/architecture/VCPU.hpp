#pragma once

#include __VCPU_HEADER

namespace DEPOS {

class VCPU : public __ARCH::VCPU {
    using __ARCH::VCPU::VCPU;
};

} // namespace DEPOS
