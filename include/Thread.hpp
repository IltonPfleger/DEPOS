#pragma once

#include <Scheduler.hpp>
#include <Traits.hpp>

class Thread {
   public:
    enum class State { RUNNING, READY, WAITING, FINISHED };
    using Criterion = typename Traits::Scheduler<Thread>::Criterion;
    using Argument  = void *;
    using Function  = int (*)(Argument);
    using Queue     = FIFO<Thread *>;
    using Element   = Queue::Node;

    ~Thread();
    Thread(Function, Argument, Criterion);

    static Thread *running();
    static void join(Thread &);
    static void exit();
    static void init();
    static void run();
    static void sleep(Queue &, Spin &);
    static void wakeup(Queue &);
    static void yield();
    static void dispatch(Thread *, Thread *, Spin *);
    static void reschedule();
    static int idle(void *);

   private:
    char *stack;
    CPU::Context *context;
    volatile State state;
    Thread *volatile joining;
    Criterion criterion;
    Element link;
    Queue *waiting;
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
