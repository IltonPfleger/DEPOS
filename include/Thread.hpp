#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>

struct Thread {
    typedef FIFO<Thread *> List;
    enum Priority { HIGH, NORMAL, LOW, IDLE = ~0 };
    enum State { RUNNING, READY, WAITING, FINISHED };

    void *stack;
    CPU::Context *context;
    Thread *joining;
    List *waiting;
    State state;
    Priority rank;

    ~Thread();
    Thread(int (*)(void *), void *, Priority);

    static inline int _count;
    static inline int _lock;
    static inline volatile Thread *_running;
    static inline Scheduler<Thread> _scheduler;
    static inline void save() { _running->context = CPU::Context::get(); };
    static inline void load() { CPU::Context::set(_running->context); };

    static void join(Thread *);
    static void exit();
    static void init();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void reschedule();
};

struct RT_Thread : Thread {
    typedef unsigned long Period;

    Period period;
    Period last;

    RT_Thread(int (*)(void *), void *, Period);
    static void wait_next();
};
