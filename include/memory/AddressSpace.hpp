#pragma once

#include <Machine.hpp>

class AddressSpace : public Machine::MMU::PageTable {
    // private:
    //  using Machine::MMU::A;
    //  using Machine::MMU::D;
    //  using Machine::MMU::R;
    //  using Machine::MMU::U;
    //  using Machine::MMU::V;
    //  using Machine::MMU::W;
    //  using Machine::MMU::X;

   public:
    static constexpr Flags UserRO   = A | D | R | U | V | X;
    static constexpr Flags UserRW   = A | D | R | U | V | W | X;
    static constexpr Flags KernelRO = A | D | R | V | X;
    static constexpr Flags KernelRW = A | D | R | V | W | X;
    static constexpr Flags Default  = UserRW;
    AddressSpace() {}
    void load() { Machine::MMU::set(reinterpret_cast<uintptr_t>(this)); }
};
