#pragma once

#include <Machine.hpp>
#include <Resource.hpp>

class AddressSpace : public SystemResource<AddressSpace, Traits::System::MULTITASK> {
   public:
    AddressSpace() {}  // Receive Kernel Page Table Or Mount I Dont Know

   public:
    bool attach(uintptr_t addr) { return pt.map(addr, addr); }

   private:
    Machine::MMU::PageTable pt;
};
