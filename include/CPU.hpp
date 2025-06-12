#ifndef CPU_H
#define CPU_H
#include <utils/definitions.hpp>

struct CPU {
    static uint8_t stack[Machine::CPUS * Machine::Memory::STACK_SIZE];

    static uint32_t id();
    static void idle();
    static void begin_atomic();
    static void end_atomic();
    __attribute__((naked)) static void save();
    __attribute__((naked)) static void load();
};

#endif
