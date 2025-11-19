#include <Alarm.hpp>
#include <Thread.hpp>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>
#include <proxys/Syscall.hpp>
#include <utils/Debug.hpp>

Thread *Thread::running() { return s_scheduler.current(); }

void Thread::dispatch(Thread *previous, Thread *next, Spin *lock) {
    next->m_state = State::RUNNING;
    lock->release();
    if (next != previous) {
        // if constexpr (Traits<System>::MULTITASK) {
        //     // next->task_->attach(previous->stack_);
        //     // next->task_->load();
        // }

        CPU::Atomic::wait(next->m_context);
        CPU::Context::swtch(const_cast<CPU::Context **>(&previous->m_context),
                            CPU::Atomic::clear(next->m_context));
    }
}

int Thread::idle(void *) {
    while (s_count > Traits<Machine>::CPUS) {
        if (!s_scheduler.empty())
            yield();
    }

    CPU::Interruptions::disable();
    if (CPU::id() == Traits<Machine>::BSP)
        Console::println("*** Shutdown! ***\n");
    for (;;)
        ;
    return 0;
}

Thread::Thread(Function f, Argument a, Criterion c)
    : m_stack_(Segment(Traits<Memory>::PAGE_SIZE)), m_waiting(0),
      m_link(Element(this, c())), m_criterion(c), m_state(State::READY),
      m_joining(0), m_context(new(m_stack_.end() - sizeof(CPU::Context))
                                  CPU::Context(f, a, exit)) {
    TraceIn(this);
    s_lock.lock();
    s_scheduler.insert(&m_link);
    s_count = s_count + 1;
    s_lock.unlock();
    TraceOut();
}

// Thread::Thread(Task *t, Function f, Argument a, Criterion c) : task_(t),
// Thread(f, a, c) {}

// Thread::~Thread() {
//     s_lock.lock();
//     switch (m_state) {
//         case (State::READY):
//             s_scheduler.remove(&m_link);
//             s_count = s_count - 1;
//             break;
//         case (State::WAITING):
//             m_waiting->remove(&m_link);
//             s_count = s_count - 1;
//             break;
//         default:
//             break;
//     }
//
//     if (m_joining) {
//         m_joining->m_state = State::READY;
//         s_scheduler.insert(&m_joining->m_link);
//     }
//
//     s_lock.release();
//     Memory::kfree(stack);
// }

void Thread::join(Thread &thread) {
    auto previous = running();
    ERROR(&thread == previous, "[Thread::join] Join itself.");
    ERROR(thread.m_joining, "[Thread::join] Already joined.");

    s_lock.lock();
    if (thread.m_state == State::FINISHED) {
        s_lock.unlock();
        return;
    }

    previous->m_state = State::WAITING;
    thread.m_joining = previous;

    dispatch(previous, s_scheduler.pop(), &s_lock);
    CPU::Interruptions::enable();
}

void Thread::exit() {
    s_lock.lock();

    auto previous = running();
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
    for (int i = 0; i < Traits<Machine>::CPUS; ++i)
        new (Heap::SYSTEM) Thread(idle, 0, Criterion::IDLE);
    TraceOut()
}

void Thread::run() {
    char buffer[sizeof(Thread)];
    Thread *previous = reinterpret_cast<Thread *>(buffer);
    s_lock.acquire();
    TraceIn();
    Thread *next = s_scheduler.pop();
    dispatch(previous, next, &s_lock);
}

void Thread::reschedule() {
    if (s_scheduler.empty())
        return;

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

    s_lock.acquire();
    auto next = s_scheduler.pop();
    s_lock.release();

    dispatch(previous, next, &lock);
}

void Thread::wakeup(Queue &waiting) {
    Element *awake = waiting.next();
    ERROR(!awake, "[Thread::wakeup] Empty queue.");
    awake->value->m_state = State::READY;
    awake->value->m_waiting = nullptr;

    s_lock.acquire();
    s_scheduler.insert(awake);
    s_lock.release();
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
