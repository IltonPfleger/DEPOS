#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>

struct Thread {
    typedef FIFO<Thread *> List;
    enum Priority : unsigned { HIGH, NORMAL, LOW, IDLE = ~0U };
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
    static inline int _lock = 1;
    static inline Scheduler<Thread> _scheduler;

    static inline volatile Thread *running() { return reinterpret_cast<volatile Thread *>(CPU::id()); };
    static inline void running(Thread *t) { t->state = RUNNING, CPU::id(t); }

    __attribute__((always_inline)) static inline void save() { running()->context = CPU::Context::get(); };
    __attribute__((always_inline)) static inline void load() { CPU::Context::set(running()->context); };

    static void join(Thread *);
    static void exit();
    static void init();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void reschedule();
};

struct RT_Thread : Thread {
    typedef unsigned long Interval;
    typedef unsigned long Time;

    Interval period;
    Time deadline;

    RT_Thread(int (*)(void *), void *, Interval);
    static void wait_next();
};
