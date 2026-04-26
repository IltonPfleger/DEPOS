#pragma once

#include <Meta.hpp>
#include <types.hpp>

namespace DEPOS {

class VirtualMachine {
  public:
    virtual bool read(uintptr_t, unsigned int *) = 0;
    virtual bool write(uintptr_t, unsigned int)  = 0;
    virtual void interrupt(unsigned int id)      = 0;
    virtual ~VirtualMachine()                    = default;
};

} // namespace DEPOS
