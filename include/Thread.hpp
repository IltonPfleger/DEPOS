#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Lists.hpp>

struct Thread {
    enum Priority { IDLE = 0, LOW = 1, NORMAL = 2, HIGH = 3 };
    enum State { RUNNING, READY, WAITING, FINISHED };
    typedef FIFO<Thread *> List;

    Thread(int (*)(void *), void *, Priority);
    ~Thread();
    uintptr_t stack;
    struct CPU::Context *context;
    struct Thread *joining;
    List *waiting;
    enum State state;
    enum Priority priority;

    static void save(CPU::Context *);
    static void join(Thread *);
    static void exit();
    static void init();
    static void stop();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void timer_handler();
};
