#include <Alarm.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <utils/Debug.hpp>

Thread *Thread::running() { return s_scheduler.current(); }

template <typename Epilogue, typename... Args>
void Thread::dispatch(Thread *previous, Thread *next, Epilogue epilogue, Args... args) {
    ERROR(!previous);
    ERROR(!next);
    ERROR(next == previous);

    next->m_state = State::RUNNING;
    CPU::Context context;

    if (context.save()) {
        previous->m_context = &context;
        next->m_context->load(epilogue, previous, args...);
    }
}

int Thread::idle(void *) {
    while (s_count > Traits<CPUS>::ACTIVE) {
        // reschedule();
        yield();
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

    CPU::Interruptions::off();
    CPU::Atomic::finc(s_count);
    s_scheduler.insert(&m_link);
    CPU::Interruptions::on();

    TraceOut();
}

// Thread::~Thread() { join(this); }

// void Thread::join(Thread *thread) {
//     CPU::Interruptions::disable();
//     s_lock.acquire();
//
//     auto previous = running();
//     ERROR(thread == previous, "Join itself.");
//     ERROR(thread->m_joining, "Already joined.");
//
//     if (thread->m_state == State::FINISHED) {
//         s_lock.release();
//         CPU::Interruptions::enable();
//         return;
//     }
//
//     previous->m_state = State::WAITING;
//     thread->m_joining = previous;
//
//     dispatch(previous, s_scheduler.remove(), &s_lock);
//     CPU::Interruptions::enable();
// }

void Thread::exit() {

    CPU::Interruptions::disable();

    Thread *previous = running();
    previous->m_state = State::FINISHED;
    Link *next = s_scheduler.remove();
    //
    //    // if (previous->m_joining) {
    //    //     previous->m_joining->m_state = State::READY;
    //    //     s_scheduler.insert(&previous->m_joining->m_link);
    //    //     previous->m_joining = 0;
    //    // }
    //
    CPU::Atomic::fdec(s_count);

    dispatch(previous, next->value(), finish);
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits<CPUS>::ACTIVE; ++i)
        new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut();
}

void Thread::run() {
    unsigned char previous[sizeof(Thread)];
    Thread *next = s_scheduler.remove()->value();
    dispatch(reinterpret_cast<Thread *>(previous), next, finish);
}

void Thread::reschedule() {
    Thread *previous = running();

    Link *next = s_scheduler.remove(Criterion::NORMAL);

    if (next) {
        previous->m_state = State::READY;
        dispatch(previous, next->value(), schedule);
    }
}

void Thread::yield() {
    CPU::Interruptions::disable();
    reschedule();
    CPU::Interruptions::enable();
}

void Thread::sleep(Queue *m_waiting, Spin *spin) {
    Thread *previous = const_cast<Thread *>(running());
    previous->m_state = State::WAITING;
    previous->m_waiting = m_waiting;

    m_waiting->insert(&previous->m_link);

    Link *next = s_scheduler.remove();

    dispatch(previous, next->value(), release, spin);
}

void Thread::wakeup(Queue *m_waiting) {
    Link *link = m_waiting->remove();

    ERROR(!link);

    Thread *awake = link->value();

    awake->m_state = State::READY;

    awake->m_waiting = nullptr;

    s_scheduler.insert(&awake->m_link);
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
