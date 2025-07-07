#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Lists.hpp>

struct Thread {
    enum Priority : int { IDLE, LOW, NORMAL, HIGH };

    enum State { RUNNING, READY, WAITING, FINISHED };
    typedef FIFO<Thread *> List;

    uintptr_t stack;
    struct CPU::Context *context;
    struct Thread *joining;
    List *waiting;
    enum State state;
    enum Priority priority;

    ~Thread();
    Thread(int (*)(void *), void *, Priority);
    Priority operator()() const;

    static void save(CPU::Context *);
    static void join(Thread *);
    static void exit();
    static void init();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void timer_handler();
};
