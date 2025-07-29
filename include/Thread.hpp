#pragma once

#include <CPU.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Scheduler.hpp>
#include <Spin.hpp>

struct Thread {
    enum class State { RUNNING, READY, WAITING, FINISHED };
    using Criterion = typename Traits::Scheduler<Thread>::Criterion;
    using Argument  = void *;
    using Function  = int (*)(Argument);
    using Queue     = FIFO<Thread *>;
    using Element   = Queue::Node;

    char *stack;
    CPU::Context *volatile context;
    volatile State state;
    Thread *volatile joining;
    Criterion criterion;
    Element link;
    Queue *waiting;


    ~Thread();
    Thread(Function, Argument, Criterion);
    static inline Thread *running() { return reinterpret_cast<Thread *>(CPU::thread()); }
    static void join(Thread &);
    static void exit();
    static void init();
    static void run();
    static void sleep(Queue *);
    static void wakeup(Queue *);
    static void yield();
    static void dispatch();
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
