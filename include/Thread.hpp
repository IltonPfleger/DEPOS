#pragma once

#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <scheduler/Scheduler.hpp>

namespace DEPOS {

class Thread {
  public:
    enum class State { RUNNING, READY, WAITING, FINISHING, FINISHED };

    using Scheduler = DEPOS::Scheduler<Thread>;
    using Criterion = Scheduler::Criterion;
    using Node      = Scheduler::Node;
    using Queue     = collections::FIFO<Node>;

    using Return   = void *;
    using Argument = void *;
    using Function = Return (*)(Argument);
    using Context  = CPU::Context;

    Thread(Function, Argument = 0, Criterion = Criterion::NORMAL);
    ~Thread();

    static void init();
    static void run();
    static void sleep(Queue *, Spin *);
    static void wakeup(Queue *);
    static void yield();
    static void reschedule();
    static void onTick();
    static void join(Thread *);

  private:
    static Thread *running();
    static void dispatch(Thread *, Thread *, Spin *);
    static void entry(Function, Argument);
    static Return idle(Argument);
    static void epilogue();
    static void exit();

  private:
    Chunk m_stack;
    Chunk m_kstack;
    Queue *m_waiting;
    Node m_node;
    volatile State m_state;
    Context *m_context;

  private:
    static inline Scheduler s_scheduler;
    static inline volatile unsigned int s_count;
    static inline Thread *s_previous[Traits<CPU>::Active];
    static inline Spin *s_spin[Traits<CPU>::Active];
};

} // namespace DEPOS
