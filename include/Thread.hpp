#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>

struct Thread {
    typedef FIFO<Thread *> List;
    enum Priority { HIGH, NORMAL, LOW, IDLE = ~0 };
    enum State { RUNNING, READY, WAITING, FINISHED };

    uintptr_t stack;
    CPU::Context *context;
    Thread *joining;
    List *waiting;
    State state;
    Priority priority;

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

    static inline volatile Thread *_running;
    static inline unsigned int _count;
    static inline Scheduler<Thread> _scheduler;
};

struct RThread : Thread {
    unsigned long period;
    unsigned long duration;
    unsigned long deadline;
    unsigned long operator()() const;
};
