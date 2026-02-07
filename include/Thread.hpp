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
    static void sleep(Queue *, Spin *);
    static void wakeup(Queue *);
    static void yield();
    static void dispatch(Thread *, Thread *, Spin *);
    static void reschedule();
    static int idle(void *);

  private:
    Segment m_stack;
    Queue *m_waiting;
    Link m_link;
    Criterion m_criterion;
    volatile State m_state;
    Thread *m_joining;
    Context *volatile m_context;

  private:
    static inline Scheduler<Thread> s_scheduler;
    static inline volatile unsigned int s_count;

  private:
    static void epilogue(Thread *t, Spin *spin) {
        switch (t->m_state) {
        case State::READY:
            s_scheduler.insert(&t->m_link);
            break;
        case State::WAITING:
            t->m_waiting->insert(&t->m_link);
            if (spin) spin->release();
            break;
        case State::ZOMBIE:
            t->m_state = State::FINISHED;
            CPU::Atomic::fdec(s_count);
            break;
        default:
            break;
        }
    }
};
