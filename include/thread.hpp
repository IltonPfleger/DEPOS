#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cpu.hpp>
#include <queue.hpp>

struct Thread {
    enum State { RUNNING, READY, WAITING };
    struct CPU::Context context;
    char* stack;
    enum State state;

    static Queue<Thread> ready;
    static void init();
};

#endif
