#pragma once

#include <Heap.hpp>
#include <Machine.hpp>

class Segment {
   public:
    constexpr Segment(void* addr = nullptr) noexcept : addr(addr) {}
    constexpr explicit operator void*() const noexcept { return addr; }

   private:
    void* addr;
};

class AddressSpace : Machine::MMU::PageTable {
   public:
    AddressSpace() {}

   public:
    void* attach(void* va) {
        map(reinterpret_cast<uintptr_t>(va), reinterpret_cast<uintptr_t>(va));
        return va;
    }
    bool attach(uintptr_t va, uintptr_t pa) { return map(va, pa); }
    void load() { Machine::MMU::set(reinterpret_cast<uintptr_t>(this)); }
};
