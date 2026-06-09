#include <Thread.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

Thread *Thread::running() { return s_scheduler.current(); }

void Thread::entry(Function f, Argument a) {
    epilogue();
    CPU::Interrupt::enable();
    f(a);
};

Thread::Return Thread::idle(Argument) {
    while (s_count > Traits<CPU>::Active) {
        // if constexpr (Traits<Timer>::Enable) CPU::idle();
        reschedule();
    }

    CPU::Interrupt::disable();

    CPU::barrier();

    if (CPU::id() == Traits<CPU>::BSP) Trace("\n*** Shutdown! ***\n");

    CPU::barrier();

    Machine::shutdown();

    return 0;
}

void Thread::dispatch(Thread *previous, Thread *next, Spin *spin = 0) {
    CPU::mb();
    assert(previous);
    assert(next);
    assert(next != previous);

    s_previous[CPU::id()] = previous;
    s_spin[CPU::id()]     = spin;

    next->m_state = State::RUNNING;

    Context::swap(previous->m_context, next->m_context);

    epilogue();
}

void Thread::epilogue() {
    unsigned int core = CPU::id();
    Thread *previous  = s_previous[core];
    Spin *spin        = s_spin[core];

    switch (previous->m_state) {
        case State::READY: s_scheduler.insert(&previous->m_node); break;
        case State::WAITING:
            previous->m_waiting->insert(&previous->m_node);
            if (spin) spin->release();
            break;
        case State::FINISHING:
            previous->m_state = State::FINISHED;
            CPU::Atomic::fdec(s_count);
            break;
        default: break;
    }
}

Thread::Thread(Function f, Argument a, Criterion c)
    : m_stack(Memory::alloc(Traits<Thread>::UserStackSize), Traits<Thread>::UserStackSize),
      m_kstack(Memory::alloc(Traits<Thread>::KernelStackSize), Traits<Thread>::KernelStackSize),
      m_waiting(0),
      m_node(Node(this, c)),
      m_state(State::READY),
      m_context(m_stack, m_kstack, entry, exit, f, a) {
    TraceIn(this);

    // m_context = Context::create(m_stack, m_kstack, entry, exit, f, a);

    bool enabled = CPU::Interrupt::disable();
    CPU::Atomic::finc(s_count);
    s_scheduler.insert(&m_node);
    if (enabled) CPU::Interrupt::enable();

    TraceOut();
}

Thread::~Thread() {
    Thread::join(*this);
    Memory::free(m_stack.data(), m_stack.size());
    Memory::free(m_kstack.data(), m_kstack.size());
}

void Thread::join(Thread &joinable) {
    while (joinable.m_state != State::FINISHED) {
        reschedule();
    }
}

void Thread::exit() {
    CPU::Interrupt::disable();

    Thread *previous = running();

    Node *next        = s_scheduler.remove();
    previous->m_state = State::FINISHING;

    dispatch(previous, next->value());
}

void Thread::init() {
    TraceIn();

    for (int i = 0; i < Traits<CPU>::Active; ++i)
        new Thread(idle, 0, Criterion::IDLE);

    TraceOut();
}

void Thread::run() {
    unsigned char buffer[sizeof(Thread)] = {0};
    // unsigned char context[sizeof(Context)] = {0};
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    // previous->m_context                    = reinterpret_cast<Context *>(context);
    previous->m_state = State::FINISHED;
    Thread *next      = s_scheduler.remove()->value();
    dispatch(previous, next);
}

void Thread::onTick() { reschedule(); }

void Thread::reschedule() {
    bool enabled = CPU::Interrupt::disable();

    Thread *previous = running();

    Node *next = s_scheduler.remove(Criterion::NORMAL);

    if (next) {
        previous->m_state = State::READY;
        CPU::mb();
        dispatch(previous, next->value());
    }

    if (enabled) CPU::Interrupt::enable();
}

void Thread::yield() { Thread::reschedule(); }

void Thread::sleep(Queue *m_waiting, Spin *spin) {
    bool enabled = CPU::Interrupt::disable();

    Thread *previous = running();

    previous->m_state   = State::WAITING;
    previous->m_waiting = m_waiting;

    Node *next = s_scheduler.remove();

    assert(next);

    dispatch(previous, next->value(), spin);

    if (enabled) CPU::Interrupt::enable();
}

void Thread::wakeup(Queue *m_waiting) {
    Node *node = m_waiting->remove();
    ERROR(!node);
    Thread *awake    = node->value();
    awake->m_state   = State::READY;
    awake->m_waiting = nullptr;

    bool enabled = CPU::Interrupt::disable();
    s_scheduler.insert(&awake->m_node);
    if (enabled) CPU::Interrupt::enable();
}

} // namespace DEPOS
