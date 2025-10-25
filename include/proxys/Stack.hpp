#pragma once
#include <Machine>
#include <Traits.hpp>
#include <proxys/Proxy.hpp>

class Stack {
    template <typename, typename...>
    friend class Proxy;

    __attribute__((naked)) static void in() {
        auto core   = Machine::CPU::core();
        _last[core] = Machine::CPU::sp();
        sp(_system[core]);
    };
    static void out() {};

    static char _system[Traits::Machine::CPUS][Traits::Memory::Page::SIZE];
    static char* _last[Traits::Machine::CPUS];
};
