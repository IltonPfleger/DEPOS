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

    // public:
    //  using Flags                     = Machine::MMU::Flags;
    //  static constexpr Flags UserRO   = A | D | R | U | V | X;
    //  static constexpr Flags UserRW   = A | D | R | U | V | W | X;
    //  static constexpr Flags KernelRO = A | D | R | V | X;
    //  static constexpr Flags KernelRW = A | D | R | V | W | X;

  public:
    AddressSpace() {}
    // void* map(void* va) {
    //     // TODO: Implement MMU Auto map function and get the available address
    //     map(reinterpret_cast<uintptr_t>(va), reinterpret_cast<uintptr_t>(va));
    //     return va;
    // }

    void load() { Machine::MMU::set(reinterpret_cast<uintptr_t>(this)); }
};
