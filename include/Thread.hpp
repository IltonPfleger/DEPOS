#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Lists.hpp>

struct Thread {
    typedef FIFO<Thread *> List;
    enum Priority { HIGH, NORMAL, LOW, IDLE = ~0 };
    enum State { RUNNING, READY, WAITING, FINISHED };

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

struct RThread : Thread {
    unsigned long period;
    unsigned long duration;
    unsigned long deadline;
    unsigned long operator()() const;
};
