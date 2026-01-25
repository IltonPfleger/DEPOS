#pragma once

#include <machine/Traits.hpp>

class MemoryMap {
  public:
    struct Entry {
        unsigned long start;
        unsigned long end;
    };

    struct App : Entry {
        unsigned long main;
        Entry text;
        Entry data;
        Entry rodata;
        Entry bss;
    };

    static void init() {
        __bmm.start = Traits<MemoryMap>::RamEnd;
        __bmm.end = Traits<MemoryMap>::RamEnd;
    }

    static inline Entry __bmm;
};

__attribute__((section(".__kernel_mm__"))) inline MemoryMap::App __kmm;
__attribute__((section(".__app_mm__"))) inline MemoryMap::App __mm;
