#pragma once

#include <architecture/CPU.hpp>
#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <utils/string.hpp>

namespace DEPOS {

class BSS {
  public:
    static void init() {
        if (CPU::id() == Traits<CPU>::BSP) {
            uintptr_t start = Memory::virt2phys(__kmm.bss.start);
            uintptr_t end = Memory::virt2phys(__kmm.bss.end);
            memset(reinterpret_cast<void *>(start), 0, end - start);
        }
        CPU::barrier();
    }
};

} // namespace DEPOS
