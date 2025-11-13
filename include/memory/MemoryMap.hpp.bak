#pragma once

struct MemoryMap {
    struct Entry {
        unsigned long start;
        unsigned long end;
    };

    struct App : Entry {
        uintptr_t main;
        Entry text;
        Entry data;
        Entry rodata;
        Entry bss;
    };
};

__attribute__((section(".__kernel_mm__"))) inline MemoryMap::App __kmm;
__attribute__((section(".__app_mm__"))) inline MemoryMap::App __mm;
