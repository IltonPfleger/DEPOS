#pragma once

#include <ELF.hpp>
#include <Traits.hpp>

extern "C" const char __KERNEL_END__[];

class Applications {
   public:
    static uintptr_t start() { return reinterpret_cast<uintptr_t>(const_cast<char*>(__KERNEL_END__)); }
    static uintptr_t end() {
        ELF* elf = reinterpret_cast<ELF*>(start());
        while (elf->valid()) {
            elf = reinterpret_cast<ELF*>(reinterpret_cast<uintptr_t>(elf) + elf->size());
        }
        return reinterpret_cast<uintptr_t>(elf);
    };

    static void init() {
        ELF* elf = reinterpret_cast<ELF*>(start());
        while (elf->valid()) {
            size_t size = elf->size();
            elf         = reinterpret_cast<ELF*>(reinterpret_cast<uintptr_t>(elf) + size);
        }
    };
};
