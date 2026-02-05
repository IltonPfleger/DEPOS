#pragma once

#include <Scheduler.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <memory/Segment.hpp>

class Thread {
  public:
    enum class State { RUNNING, READY, WAITING, FINISHED, ZOMBIE };
    using Criterion = typename Scheduler<Thread>::Criterion;
    using Argument = void *;
    using Function = int (*)(Argument);
    using Link = Node<Thread *, Criterion>;
    using Queue = FIFO<Link>;
    using Context = CPU::Context;

    Thread() = default;
    Thread(Function, Argument, Criterion);
    ~Thread();

    static Thread *running();
    static void join(Thread *);
    static void exit();
    static void init();
    static void run();
    static void sleep(Queue &);
    static void wakeup(Queue &);
    static void yield();
    template <typename Epilogue, typename... Args> static void dispatch(Thread *, Thread *, Epilogue, Args...);
    static void reschedule();
    static int idle(void *);

  private:
    Segment m_stack;
    Queue *m_waiting;
    Link m_link;
    Criterion m_criterion;
    volatile State m_state;
    Thread *volatile m_joining;
    Context *volatile m_context;

  private:
    static inline Scheduler<Thread> s_scheduler;
    static inline volatile unsigned int s_count;
    static inline Spin s_lock;

  private:
    static void schedule(Thread *t) {
        if (t->m_state != State::ZOMBIE) {
            s_scheduler.insert(&t->m_link);
        }
    }

    static void enqueue(Thread *t, Queue *q) { q->insert(&t->m_link); };
};
