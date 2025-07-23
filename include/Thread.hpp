#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>

struct Thread {
    using Criterion = Traits<Scheduler<Thread>>::Criterion;
    using Function  = int (*)(void *);
    using Argument  = void *;
    using Element   = Scheduler<Thread>::Queue::Element;
    using Queue     = FIFO<Thread *>;
    enum class State { RUNNING, READY, WAITING, FINISHED };
    enum : Criterion::Rank { HIGH, NORMAL, LOW, IDLE = ~0ULL };

    void *stack;
    State state;
    CPU::Context *context;
    Thread *joining;
    Queue *waiting;
    Element *link;
    Criterion criterion;

    ~Thread();
    Thread(Function, Argument, Criterion);

    static inline int _count;
    static inline Scheduler<Thread> _scheduler;

    static inline volatile Thread *running() { return reinterpret_cast<volatile Thread *>(CPU::id()); };
    static inline void running(Thread *t) { t->state = State::RUNNING, CPU::id(t); }

    static void join(Thread *);
    static void exit();
    static void init();
    static void core();
    static void sleep(Queue *);
    static void wakeup(Queue *);
    static void yield();
    static void reschedule();
};

struct RT_Thread : Thread {
    const Function function;
    const Microsecond deadline;
    const Microsecond period;
    const Microsecond duration;
    Microsecond start;

    RT_Thread(Function, Argument, Microsecond, Microsecond, Microsecond, Microsecond);
};
