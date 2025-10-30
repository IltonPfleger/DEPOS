#pragma once

#include <Initializer.hpp>
#include <Machine.hpp>

class AddressSpace : Initializer<AddressSpace> {
   private:
    AddressSpace() {}  // Receive Kernel Page Table Or Mount I Dont Know

   public:
    bool attach(uintptr_t addr) { return pt.map(addr, addr); }

   private:
    Machine::MMU::PageTable pt;
};
