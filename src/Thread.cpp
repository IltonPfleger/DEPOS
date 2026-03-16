#include <Alarm.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

Thread *Thread::running() { return s_scheduler.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *spin = 0) {
    CPU::mb();
    ERROR(!previous);
    ERROR(!next);
    ERROR(next == previous);

    CPU::Context context;
    previous->m_context = &context;
    next->m_state       = State::RUNNING;

    Context::swap(previous->m_context, next->m_context, epilogue, previous, spin);
}

Thread::Return Thread::idle(Argument) {
    while (s_count > Traits<CPU>::Active) {
        reschedule();
    }

    CPU::Interruptions::disable();
    CPU::barrier();
    if (CPU::id() == Traits<CPU>::BSP) Console::cout << "\n*** Shutdown! ***\n";
    CPU::halt();
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : m_stack(Traits<Thread>::UserStackSize),
      m_kstack(Traits<Thread>::KernelStackSize),
      m_waiting(0),
      m_criterion(c),
      m_link(Link(this, m_criterion)),
      m_state(State::READY) {

    TraceIn(this);

    m_context  = reinterpret_cast<Context *>(m_stack.end() - sizeof(Context));
    *m_context = Context(m_stack.end(), m_kstack.end(), f, exit, a);

    bool enabled = CPU::Interruptions::disable();
    CPU::Atomic::finc(s_count);
    s_scheduler.insert(&m_link);
    if (enabled) CPU::Interruptions::enable();

    TraceOut();
}

Thread::~Thread() { Thread::join(this); }

void Thread::join(Thread *joinable) {
    while (joinable->m_state != State::FINISHED) {
        reschedule();
    }
}

void Thread::exit() {
    CPU::Interruptions::disable();

    Thread *previous = running();

    Link *next        = s_scheduler.remove();
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
    Thread *previous  = reinterpret_cast<Thread *>(buffer);
    previous->m_state = State::FINISHED;
    Thread *next      = s_scheduler.remove()->value();
    dispatch(previous, next);
}

void Thread::reschedule() {
    bool enabled = CPU::Interruptions::disable();

    Thread *previous = running();

    Link *next = s_scheduler.remove(Criterion::NORMAL);

    if (next) {
        previous->m_state = State::READY;
        CPU::mb();
        dispatch(previous, next->value());
    }

    if (enabled) CPU::Interruptions::enable();
}

void Thread::sleep(Queue *m_waiting, Spin *spin) {
    bool enabled = CPU::Interruptions::disable();

    Thread *previous = running();

    previous->m_state   = State::WAITING;
    previous->m_waiting = m_waiting;

    Link *next = s_scheduler.remove();

    dispatch(previous, next->value(), spin);

    if (enabled) CPU::Interruptions::enable();
}

void Thread::wakeup(Queue *m_waiting) {
    Link *link = m_waiting->remove();
    ERROR(!link);
    Thread *awake    = link->value();
    awake->m_state   = State::READY;
    awake->m_waiting = nullptr;

    bool enabled = CPU::Interruptions::disable();
    s_scheduler.insert(&awake->m_link);
    if (enabled) CPU::Interruptions::enable();
}

} // namespace DEPOS
