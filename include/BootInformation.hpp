#pragma once

#include <memory/Chunk.hpp>
#include <types.hpp>

namespace QUARK {

struct BootInformation {
    struct Payload : Chunk {
        uintptr_t main;
        Chunk text;
        Chunk data;
        Chunk rodata;
        Chunk bss;
    };
};

__attribute__((section(".__kernel_mm__"))) inline BootInformation::Payload __kmm;
__attribute__((section(".__payload_mm__"))) inline BootInformation::Payload __mm;
__attribute__((section(".__all_mm__"))) inline Chunk __amm;
__attribute__((section(".__boot_mm__"))) inline Chunk __bmm;

} // namespace QUARK
