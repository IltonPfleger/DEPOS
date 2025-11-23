#pragma once

#include <Scheduler.hpp>
#include <Spin.hpp>
#include <Task.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <memory/Segment.hpp>

class Thread {
  public:
    enum class State { RUNNING, READY, WAITING, FINISHED };
    using Criterion = typename Scheduler<Thread>::Criterion;
    using Argument = void *;
    using Function = int (*)(Argument);
    using Queue = LIFO<Element<Thread *, Criterion>>;
    using Node = Queue::Node;

    // Thread(Function f, Argument a, Criterion c, Task *t = nullptr)
    //     : // task_(t ? t : new(Heap::SYSTEM) Task()),
    //       m_stack_(Segment(Traits<Memory>::PAGE_SIZE)),
    //       um_stack_(Segment(Traits<Memory>::PAGE_SIZE)), waiting(0),
    //       m_link(Element(this, c())), criterion(c), state(State::RUNNING),
    //       joining(0), context_(new(m_stack_.end() - sizeof(CPU::Context))
    //                                CPU::Context(f, a, exit)) {
    //     TraceIn(this);
    //     (void)t;
    //     // task_->attach(m_stack_, Task::AddressSpace::Flags::KernelRW);
    //     // task_->attach(um_stack_, Task::AddressSpace::Flags::UserRW);
    //     lock_s.lock();
    //     scheduler_s.insert(&m_link);
    //     count_s = count_s + 1;
    //     lock_s.unlock();
    //     TraceOut();
    // }

    // template <typename T = void>
    //     requires(!Traits<System>::MULTITASK)
    Thread(Function, Argument, Criterion);
    ~Thread();

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
    Segment m_stack_;
    Queue *m_waiting;
    Queue::Node m_link;
    Criterion m_criterion;
    volatile State m_state;
    Thread *volatile m_joining;
    CPU::Context *volatile m_context;

  private:
    static inline Scheduler<Thread> s_scheduler;
    static inline volatile unsigned int s_count;
    static inline Spin s_lock;
};

// struct RT_Thread : Thread {
//     const Function function;
//     const Microsecond deadline;
//     const Microsecond period;
//     const Microsecond duration;
//     Microsecond start;
//
//     RT_Thread(Function, Argument, Microsecond, Microsecond, Microsecond,
//     Microsecond);
// };
