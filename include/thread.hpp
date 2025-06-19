#ifndef THREAD_HPP
#define THREAD_HPP

#include <cpu.hpp>
#include <definitions.hpp>

struct Thread {
    typedef int (*ThreadEntry)(void*);
    enum Priority {
        HIGH = 0,
        NORMAL,
        LOW,
        LAST,
    };
    enum State {
        RUNNING,
        READY,
        WAITING,
    };

    Thread(ThreadEntry);
    static void exit();
    static void dispatch(Thread*, Thread*);

    uintptr_t stack;
    struct CPU::Context context;
    enum State state;
};

#endif
