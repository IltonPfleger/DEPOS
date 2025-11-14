#pragma once

#include <Machine.hpp>
#include <Scheduler.hpp>
#include <Task.hpp>
#include <Traits.hpp>
#include <memory/Segment.hpp>

class Thread {
   public:
    enum class State { RUNNING, READY, WAITING, FINISHED };
    using Criterion = typename Scheduler<Thread>::Criterion;
    using Argument  = void *;
    using Function  = int (*)(Argument);
    using Queue     = FIFO<Thread *>;
    using Element   = Queue::Node;
    using CPU       = Machine::CPU;

    Thread(Function f, Argument a, Criterion c, Task *t = nullptr)
        : task_(t ? t : new(Heap::SYSTEM) Task()),
          stack_(Segment(Traits<Memory>::PAGE_SIZE)),
          ustack_(Segment(Traits<Memory>::PAGE_SIZE)),
          waiting(0),
          link(Element(this, c())),
          criterion(c),
          state(State::RUNNING),
          joining(0),
          context_(new(stack_.end() - sizeof(CPU::Context)) CPU::Context(f, a, exit)) {
        TraceIn(this);
        task_->attach(stack_, Task::AddressSpace::Flags::KernelRW);
        task_->attach(ustack_, Task::AddressSpace::Flags::UserRW);
        lock_.lock();
        scheduler_.insert(&link);
        count_ = count_ + 1;
        lock_.unlock();
        TraceOut();
    }

    template <typename T = void>
        requires(!Traits<System>::MULTITASK)
    Thread(Function f, Argument a, Criterion c)
        : stack_(Segment(Traits<Memory>::PAGE_SIZE)),
          waiting(0),
          link(Element(this, c())),
          criterion(c),
          state(State::READY),
          joining(0),
          context_(new(stack_.end() - sizeof(CPU::Context)) CPU::Context(f, a, exit)) {
        TraceIn(this);
        lock_.lock();
        scheduler_.insert(&link);
        count_ = count_ + 1;
        lock_.unlock();
        TraceOut();
    }

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
    Task *task_;
    Segment stack_;
    Segment ustack_;
    Queue *waiting;
    Queue::Node link;
    Criterion criterion;
    volatile State state;
    Thread *volatile joining;
    CPU::Context *volatile context_;

   private:
    static inline Scheduler<Thread> scheduler_;
    static inline volatile unsigned int count_;
    static inline Spin lock_;
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
