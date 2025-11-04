#pragma once

#include <Machine.hpp>

class AddressSpace : Machine::MMU::PageTable {
    // public:
    //  AddressSpace() {}

    // public:
    //  using Machine::MMU::PageTable::map;

    // void* map(void* va) {
    //     // TODO: Implement MMU Auto map function and get the available address
    //     map(reinterpret_cast<uintptr_t>(va), reinterpret_cast<uintptr_t>(va));
    //     return va;
    // }

    // void load() { Machine::MMU::set(reinterpret_cast<uintptr_t>(this)); }
};
