#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>
#include <Spin.hpp>

struct Thread {
    using Criterion = Traits::Scheduler<Thread>::Criterion;
    using Function  = int (*)(void *);
    using Argument  = void *;
    using Element   = Scheduler<Thread>::Queue::Element;
    using Queue     = FIFO<Thread *>;
    enum class State { RUNNING, READY, WAITING, FINISHED };
    enum : Criterion::Rank { IDLE, NORMAL };

    char *stack;
    CPU::Context *volatile context;
    volatile State state;
    Thread *volatile joining;
    Criterion criterion;
    Element link;
    Queue *waiting;

    ~Thread();
    Thread(Function, Argument, Criterion);

    static inline volatile int _count;
    static inline Scheduler<Thread> _scheduler;

    static inline Thread *running() { return reinterpret_cast<Thread *>(CPU::thread()); }

    static inline Spin spin;
    static void lock() {
        CPU::Interrupt::disable();
        spin.lock();
    }
    static void unlock() { spin.unlock(); }

    static void join(Thread *);
    static void exit();
    static void init();
    static void run();
    static void sleep(Queue *);
    static void wakeup(Queue *);
    static void yield();
    static void reschedule();

    static int idle(void *);
};

// struct RT_Thread : Thread {
//     const Function function;
//     const Microsecond deadline;
//     const Microsecond period;
//     const Microsecond duration;
//     Microsecond start;
//
//     RT_Thread(Function, Argument, Microsecond, Microsecond, Microsecond, Microsecond);
// };
