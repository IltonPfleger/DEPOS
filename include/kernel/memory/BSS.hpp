#pragma once

#include <BootInformation.hpp>
#include <architecture/CPU.hpp>
#include <kernel/memory/Memory.hpp>
#include <shared/libraries/libc/string.h>

namespace DEPOS {

class BSS {
  public:
    static void init() {
        char *start = (char *)Memory::virt2phys(__kmm.bss.start());
        char *end   = (char *)Memory::virt2phys(__kmm.bss.end());
        memset(start, 0, end - start);
        start = (char *)Memory::virt2phys(__mm.bss.start());
        end   = (char *)Memory::virt2phys(__mm.bss.end());
        memset(start, 0, end - start);
        CPU::barrier();
    }
};

} // namespace DEPOS
