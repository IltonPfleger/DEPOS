#ifndef cpu_HPP
#define cpu_HPP
#include <definitions.hpp>

struct CPU {
    static uint8_t stack[Machine::CPUS][Machine::Memory::PAGE_SIZE];

    static uint32_t id();
    static void idle();
    static void begin_atomic();
    static void end_atomic();
    __attribute__((naked)) static void save();
    __attribute__((naked)) static void load();
};

#endif
