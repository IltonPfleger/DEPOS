#pragma once

#ifndef __VCPU_HEADER
#error "VCPU: Not Implemented For This Architecture."
#endif

#include __VCPU_HEADER

namespace DEPOS {

class VCPU : public __ARCH::VCPU {
    using __ARCH::VCPU::VCPU;
};

} // namespace DEPOS
