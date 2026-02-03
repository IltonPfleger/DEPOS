#pragma once

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
};

__attribute__((section(".__kernel_mm__"))) inline MemoryMap::App __kmm;
__attribute__((section(".__app_mm__"))) inline MemoryMap::App __mm;
__attribute__((section(".__boot_mm__"))) volatile inline MemoryMap::Entry __bmm;
//= {Traits<MemoryMap>::RamEnd, Traits<MemoryMap>::RamEnd};
