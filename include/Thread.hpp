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

    static inline volatile Thread *_running;
    static inline unsigned int _count;
    static inline Scheduler<Thread> _scheduler;

    static void save(CPU::Context *);
    static void join(Thread *);
    static void exit();
    static void init();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void timer_handler();
    static inline void save() { _running->context = CPU::Context::get(); };
    static inline void load() { CPU::Context::set(_running->context); };
};

struct RThread : Thread {
    unsigned long period;
    unsigned long duration;
    unsigned long deadline;
    unsigned long operator()() const;
};
