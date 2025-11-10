#pragma once

#include <Machine.hpp>

class Syscall {
    template <auto Function, auto... Args> static auto call() {
        ((void)Args, ...);
        Machine::CPU::ecall();
    }
};
