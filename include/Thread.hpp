#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>

struct Thread {
    typedef FIFO<Thread *> List;
    typedef uintptr_t Rank;
    enum class State { RUNNING, READY, WAITING, FINISHED };
    enum { HIGH, NORMAL, LOW, IDLE = ~0ULL };

    void *stack;
    State state;
    Rank rank;
    CPU::Context *context;
    Thread *joining;
    List *waiting;

    ~Thread();
    Thread(int (*)(void *), void *, Rank);

    static inline int _count;
    static inline int _lock = 1;
    static inline Scheduler<Thread> _scheduler;

    static inline volatile Thread *running() { return reinterpret_cast<volatile Thread *>(CPU::id()); };
    static inline void running(Thread *t) { t->state = State::RUNNING, CPU::id(t); }

    static void join(Thread *);
    static void exit();
    static void init();
    static void sleep(List *);
    static void wakeup(List *);
    static void yield();
    static void reschedule();
};

struct RT_Thread : Thread {
    typedef unsigned Interval;
    typedef Interval Duration;
    typedef uintptr_t Time;

    Interval &period = reinterpret_cast<Interval &>(rank);
    Time deadline;

    RT_Thread(int (*)(void *), void *, Interval);
    static void wait_next();
};
