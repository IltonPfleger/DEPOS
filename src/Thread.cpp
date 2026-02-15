#include <Alarm.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <utils/Debug.hpp>

Thread *Thread::running() { return s_scheduler.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *spin = 0) {
    ERROR(!previous);
    ERROR(!next);
    ERROR(next == previous);

    next->m_state = State::RUNNING;
    CPU::Context context;

    if (context.save()) {
        previous->m_context = &context;
        next->m_context->load(epilogue, previous, spin);
    }
}

int Thread::idle(void *) {
    while (s_count > Traits<CPU>::Active) {
        yield();
    }

    CPU::Interruptions::disable();
    CPU::barrier();
    if (CPU::id() == Traits<CPU>::BSP) Console::cout << "\n*** Shutdown! ***\n";
    CPU::halt();
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : m_stack(Segment(Traits<Memory>::PageSize)), m_waiting(0), m_link(Link(this, c)), m_criterion(c), m_state(State::READY),
      m_context(new(m_stack.end() - sizeof(CPU::Context)) CPU::Context(f, a, m_stack.end(), exit)) {

    TraceIn(this);

    bool enabled = CPU::Interruptions::disable();
    CPU::Atomic::finc(s_count);
    s_scheduler.insert(&m_link);
    if (enabled) CPU::Interruptions::enable();

    TraceOut();
}

Thread::~Thread() { ERROR(m_state != State::FINISHED); }

void Thread::exit() {
    Thread *previous = running();

    CPU::Interruptions::disable();

    Link *next = s_scheduler.remove();
    previous->m_state = State::FINISHING;

    dispatch(previous, next->value());
}

void Thread::init() {
    TraceIn();
    for (int i = 0; i < Traits<CPU>::Active; ++i)
        new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut();
}

void Thread::run() {
    unsigned char buffer[sizeof(Thread)];
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    previous->m_state = State::FINISHED;
    Thread *next = s_scheduler.remove()->value();
    TraceIn(next);
    dispatch(previous, next);
}

void Thread::reschedule() {
    Thread *previous = running();

    Link *next = s_scheduler.remove(Criterion::NORMAL);

    if (next) {
        previous->m_state = State::READY;
        dispatch(previous, next->value());
    }
}

void Thread::yield() {
    CPU::Interruptions::disable();
    reschedule();
    CPU::Interruptions::enable();
}

void Thread::sleep(Queue *m_waiting, Spin *spin) {
    Thread *previous = running();

    previous->m_state = State::WAITING;
    previous->m_waiting = m_waiting;

    Link *next = s_scheduler.remove();

    dispatch(previous, next->value(), spin);
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
