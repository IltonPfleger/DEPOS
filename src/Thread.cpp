#include <Alarm.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <utils/Debug.hpp>

Thread *Thread::running() { return s_scheduler.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *lock) {
    next->m_state = State::RUNNING;
    CPU::Context context;

    if (next == previous) {
        lock->release();
        return;
    }
    if (context.save()) {
        previous->m_context = &context;
        next->m_context->load(Meta::Caller<Spin, &Spin::release>::Result, lock);
    }
}

int Thread::idle(void *) {
    while (s_count > Traits<CPUS>::ACTIVE) {
        if (!s_scheduler.empty()) yield();
    }

    CPU::Interruptions::disable();
    CPU::barrier();
    if (CPU::id() == Traits<CPUS>::BSP) Console::println("*** Shutdown! ***\n");
    CPU::halt();
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : m_stack(Segment(Traits<Memory>::PageSize)), m_waiting(0), m_link(Link(this, c)), m_criterion(c), m_state(State::READY),
      m_joining(0), m_context(new(m_stack.end() - sizeof(CPU::Context)) CPU::Context(f, a, m_stack.end(), exit)) {
    TraceIn(this);

    CPU::Interruptions::disable();
    s_lock.acquire();

    s_scheduler.insert(&m_link);
    s_count = s_count + 1;

    s_lock.release();
    CPU::Interruptions::enable();

    TraceOut();
}

void Thread::join(Thread *thread) {
    CPU::Interruptions::disable();
    s_lock.acquire();

    auto previous = running();
    ERROR(thread == previous, "Join itself.");
    ERROR(thread->m_joining, "Already joined.");

    if (thread->m_state == State::FINISHED) {
        s_lock.release();
        CPU::Interruptions::enable();
        return;
    }

    previous->m_state = State::WAITING;
    thread->m_joining = previous;

    dispatch(previous, s_scheduler.pop(), &s_lock);
    CPU::Interruptions::enable();
}

void Thread::exit() {
    CPU::Interruptions::disable();

    s_lock.acquire();

    Thread *previous = running();

    previous->m_state = State::FINISHED;

    if (previous->m_joining) {
        previous->m_joining->m_state = State::READY;
        s_scheduler.insert(&previous->m_joining->m_link);
        previous->m_joining = 0;
    }

    s_count = s_count - 1;

    dispatch(previous, s_scheduler.pop(), &s_lock);
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits<CPUS>::ACTIVE; ++i)
        new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut();
}

void Thread::run() {
    unsigned char previous[sizeof(Thread)];
    s_lock.acquire();
    Thread *next = s_scheduler.pop();
    dispatch(reinterpret_cast<Thread *>(previous), next, &s_lock);
}

void Thread::reschedule() {
    if (s_scheduler.empty()) return;

    auto previous = running();
    previous->m_state = State::READY;

    s_lock.acquire();
    s_scheduler.insert(&previous->m_link);
    dispatch(previous, s_scheduler.pop(), &s_lock);
}

void Thread::yield() {
    CPU::Interruptions::disable();
    reschedule();
    CPU::Interruptions::enable();
}

void Thread::sleep(Queue &m_waiting, Spin &lock) {

    auto previous = running();
    previous->m_state = State::WAITING;
    previous->m_waiting = &m_waiting;
    m_waiting.insert(&previous->m_link);

    CPU::Interruptions::disable();
    s_lock.acquire();
    auto next = s_scheduler.pop();
    s_lock.release();
    dispatch(previous, next, &lock);
}

void Thread::wakeup(Queue &waiting) {
    Link *link = waiting.remove();
    ERROR(!link, "Empty queue.");
    Thread *awake = link->value();
    awake->m_state = State::READY;
    awake->m_waiting = nullptr;

    CPU::Interruptions::disable();
    s_lock.acquire();
    s_scheduler.insert(link);
    s_lock.release();
    CPU::Interruptions::enable();
}

// int entry(void *arg) {
//     RT_Thread *current = const_cast<RT_Thread *>(static_cast<volatile
//     RT_Thread *>(Thread::running())); auto now           = Alarm::utime(); if
//     (now < current->start) Alarm::usleep(current->start - now);
//
//     while (1) {
//         current->function(arg);
//         now = Alarm::utime();
//
//         int miss = (now - current->start) - current->deadline;
//         ERROR(miss > 0, "Missed deadline: %dμ\n", miss);
//
//         current->start += current->period;
//         Alarm::usleep(current->start - now);
//     }
//     return 0;
// }
//
// RT_Thread::RT_Thread(Function f, Argument a, Microsecond d, Microsecond p,
// Microsecond c, Microsecond s)
//     : Thread(entry, a, Criterion(d, p, c)), function(f), deadline(d),
//     period(p), duration(c), start(s) {}
