#pragma once

#include <shared/memory/Chunk.hpp>
#include <types.hpp>

namespace DEPOS {

struct BootInformation {
    struct Application : Chunk {
        Application() {};

        uintptr_t main;
        Chunk text;
        Chunk data;
        Chunk rodata;
        Chunk bss;
    };
};

__attribute__((section(".__kernel_mm__"))) inline BootInformation::Application __kmm;
__attribute__((section(".__app_mm__"))) inline BootInformation::Application __mm;
__attribute__((section(".__all_mm__"))) inline Chunk __amm;
__attribute__((section(".__boot_mm__"))) inline Chunk __bmm;

} // namespace DEPOS
