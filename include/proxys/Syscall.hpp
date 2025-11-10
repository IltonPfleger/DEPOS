#pragma once

#include <Machine.hpp>

class Syscall {
  public:
    template <auto Function, auto... Args> static auto call() { Machine::CPU::syscall(Function, Args...); }
};
