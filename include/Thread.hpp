#pragma once

#include <Scheduler.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <memory/Segment.hpp>

namespace DEPOS {

class Thread {
  public:
    enum class State { RUNNING, READY, WAITING, FINISHING, FINISHED };
    using Criterion = typename Scheduler<Thread>::Criterion;
    using Return = void *;
    using Argument = void *;
    using Function = Return (*)(Argument);
    using Link = Node<Thread *, Criterion>;
    using Queue = FIFO<Link>;
    using Context = CPU::Context;

    Thread(Function, Argument, Criterion = Criterion::NORMAL);
    ~Thread();

    static Thread *running();
    static void exit();
    static void init();
    static void run();
    static void sleep(Queue *, Spin *);
    static void wakeup(Queue *);
    static void yield();
    static void join(Thread *);
    static void dispatch(Thread *, Thread *, Spin *);
    static void reschedule();
    static Return idle(Argument);

  private:
    Segment m_stack;
    Segment m_kstack;
    Queue *m_waiting;
    Link m_link;
    Criterion m_criterion;
    volatile State m_state;
    Context *m_context;

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
            spin->release();
            break;
        case State::FINISHING:
            t->m_state = State::FINISHED;
            CPU::Atomic::fdec(s_count);
            break;
        default:
            break;
        }
    }
};

} // namespace DEPOS
