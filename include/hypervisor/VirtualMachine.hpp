#pragma once

#include <Meta.hpp>
#include <hypervisor/VirtualMemoryManager.hpp>
#include <memory/Chunk.hpp>
#include <types.hpp>

namespace QUARK {

class VirtualMachine {
  public:
    VirtualMachine(const Chunk &&chunk)
        : memory_(static_cast<const Chunk &&>(chunk)) {}

    virtual bool read(uintptr_t target, unsigned int *destination) = 0;
    virtual bool write(uintptr_t target, unsigned int source)      = 0;
    virtual void interrupt(unsigned int id)                        = 0;
    virtual const VirtualMemoryManager &memory() { return memory_; }
    virtual ~VirtualMachine() = default;

  private:
    VirtualMemoryManager memory_;
};

} // namespace QUARK
