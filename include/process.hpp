#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <definitions.hpp>

struct Process {
    enum State { RUNNING, READY, BLOCKED };
    char context[Machine::Memory::Page::SIZE];
    char stack[Machine::Memory::Page::SIZE];
    enum State state;
};

#endif
